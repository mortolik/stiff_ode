#include "StiffOdeModel.hpp"
#include <QObject>
#include <QtCharts/QLineSeries>
#include <cmath>
#include <vector>
#include <QDebug>
#include <functional>
#include <Eigen/Dense>

namespace StiffOde
{
StiffOdeModel::StiffOdeModel(QObject* parent)
    : QObject(parent), m_startTime(0.0), m_endTime(0.0), m_stepSize(0.1),
    m_endExactTime(0.0), m_startExactTime(0.0)
{
    // Инициализация системы по умолчанию
    m_system = [](const std::vector<double>& y, double /*t*/) -> std::vector<double>
    {
        return
            {
                -500.005 * y[0] + 499.995 * y[1],
                499.995 * y[0] - 500.005 * y[1]
            };
    };
}

void StiffOdeModel::setSystem(const std::function<std::vector<double>(const std::vector<double>&, double)>& system)
{
    m_system = system;
}

void StiffOdeModel::setInitialConditions(const std::vector<double>& initialConditions, double startTime)
{
    m_initialConditions = initialConditions;
    m_startTime = startTime;
}

void StiffOdeModel::setParameters(double stepSize, double endTime, double endExactTime, double startExactTime)
{
    m_stepSize = stepSize;
    m_endTime = endTime;
    m_endExactTime = endExactTime;
    m_startExactTime = startExactTime;
}

void StiffOdeModel::solve()
{
    if (!m_system || m_initialConditions.empty())
        return;

    // Очистим старые данные
    qDeleteAll(m_series);
    m_series.clear();

    size_t numEquations = m_initialConditions.size();
    for (size_t i = 0; i < numEquations; ++i) {
        m_series.push_back(new QLineSeries(this));
    }

    std::vector<double> y = m_initialConditions;
    double t = m_startTime;

    const size_t maxSteps = 1e6; // ограничение на количество шагов
    size_t currentStep = 0;

    // Создаём матрицы для метода Эйлера (неявного)
    Eigen::MatrixXd A(numEquations, numEquations);
    // Заполнение матрицы A по вашей системе
    A << -500.005, 499.995,
        499.995, -500.005;

    Eigen::MatrixXd I = Eigen::MatrixXd::Identity(numEquations, numEquations);
    Eigen::MatrixXd M = I - m_stepSize * A;
    Eigen::MatrixXd M_inv = M.inverse();

    while (t <= m_endTime)
    {
        bool belowThreshold = std::all_of(y.begin(), y.end(),
                                          [](double val){ return std::abs(val) <= 1e-9; });
        if (belowThreshold) {
            qDebug() << "Stopped due to value falling below threshold at t =" << t;
            break;
        }

        if (currentStep > maxSteps) {
            qDebug() << "Stopped due to exceeding maximum number of steps.";
            break;
        }

        // Запоминаем точку (добавляем *каждый* шаг в m_series)
        for (size_t i = 0; i < numEquations; ++i) {
            m_series[i]->append(t, y[i]);
        }

        // Считаем y_{n+1} = M_inv * y_n
        Eigen::VectorXd yVec(numEquations);
        for (size_t i = 0; i < numEquations; ++i)
            yVec(i) = y[i];

        Eigen::VectorXd yNextVec = M_inv * yVec;

        for (size_t i = 0; i < numEquations; ++i)
            y[i] = yNextVec(i);

        t += m_stepSize;
        currentStep++;
    }

    qDebug() << "Численное решение завершено. Количество шагов:" << currentStep;
}

std::vector<QPointF> StiffOdeModel::computeExactSolution() const
{
    Eigen::Matrix2d A;
    A << -500.005, 499.995,
        499.995, -500.005;

    Eigen::EigenSolver<Eigen::Matrix2d> solver(A);
    Eigen::Vector2d eigenValues = solver.eigenvalues().real();
    Eigen::Matrix2d eigenVectors = solver.eigenvectors().real();

    Eigen::Vector2d initialConditions(m_initialConditions[0], m_initialConditions[1]);
    Eigen::Vector2d coefficients = eigenVectors.inverse() * initialConditions;

    std::vector<QPointF> exactSolution;

    // Проверяем, есть ли численное решение
    if (m_series.empty()) {
        qDebug() << "Численное решение отсутствует или пусто.";
        return exactSolution;
    }

    size_t numSteps = m_series[0]->count();
    const double threshold = 1e-15;

    for (size_t i = 0; i < numSteps; ++i)
    {
        double t = m_series[0]->at(i).x();

        Eigen::Vector2d solution = coefficients[0] * std::exp(eigenValues(0) * t) * eigenVectors.col(0) +
                                   coefficients[1] * std::exp(eigenValues(1) * t) * eigenVectors.col(1);

        double y0 = (std::abs(solution(0)) < threshold) ? 0.0 : solution(0);
        double y1 = (std::abs(solution(1)) < threshold) ? 0.0 : solution(1);

        exactSolution.emplace_back(t, y0);
        exactSolution.emplace_back(t, y1);
    }

    qDebug() << "Точное решение завершено. Количество шагов:" << exactSolution.size() / 2;

    return exactSolution;
}

std::vector<std::vector<QPointF>> StiffOdeModel::computeGlobalError() const
{
    const auto exactSolution = computeExactSolution();
    const auto& numericalSolution = m_series;

    if (exactSolution.empty() || numericalSolution.empty())
        return {};

    size_t numSteps = numericalSolution[0]->count();
    size_t numComponents = numericalSolution.size();

    // Теперь количество точек совпадает
    std::vector<std::vector<QPointF>> globalErrors(numComponents, std::vector<QPointF>());

    for (size_t i = 0; i < numSteps; ++i) {
        double t = numericalSolution[0]->at(i).x();

        for (size_t j = 0; j < numComponents; ++j) {
            double numericalValue = numericalSolution[j]->at(i).y();
            double exactValue = exactSolution[i * numComponents + j].y();

            double error = numericalValue - exactValue;
            globalErrors[j].emplace_back(t, error);
        }
    }

    qDebug() << "Глобальная погрешность вычислена.";

    return globalErrors;
}

double StiffOdeModel::getStepSize() {return m_stepSize;}

const std::vector<QLineSeries*>& StiffOdeModel::getSeries() const
{
    return m_series;
}
}
