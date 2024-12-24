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
    : QObject(parent), m_startTime(0.0), m_endTime(0.0), m_stepSize(0.1)
{
    m_system = [](const std::vector<double>& y, double t) -> std::vector<double>
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
    double t = m_startExactTime;

    const double threshold = 1e-15;

    while (t <= m_endExactTime)
    {
        Eigen::Vector2d solution = coefficients[0] * exp(eigenValues[0] * t) * eigenVectors.col(0) +
                                   coefficients[1] * exp(eigenValues[1] * t) * eigenVectors.col(1);

        double y0 = (std::abs(solution[0]) < threshold) ? 0.0 : solution[0];
        double y1 = (std::abs(solution[1]) < threshold) ? 0.0 : solution[1];

        exactSolution.emplace_back(t, y0);
        exactSolution.emplace_back(t, y1);

        t += m_stepSize;
    }

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

    std::vector<std::vector<QPointF>> globalErrors(numComponents);
    const double stopThreshold = 1e-09;

    for (size_t i = 0; i < numSteps; ++i) {
        double t = numericalSolution[0]->at(i).x();

        for (size_t j = 0; j < numComponents; ++j) {
            double numericalValue = numericalSolution[j]->at(i).y();
            double exactValue = exactSolution[i * numComponents + j].y();

            if (std::abs(numericalValue) <= stopThreshold || std::abs(exactValue) <= stopThreshold) {
                qDebug() << "Stopped due to value exceeding threshold at t =" << t;
                return globalErrors;
            }

            double error = numericalValue - exactValue;
            globalErrors[j].emplace_back(t, error);
        }
    }

    return globalErrors;
}

double StiffOdeModel::getExactEndTime()
{
    return m_endExactTime;
}

void StiffOdeModel::solve()
{
    if (!m_system || m_initialConditions.empty())
        return;

    size_t numEquations = m_initialConditions.size();
    const size_t maxSteps = 1e6;
    size_t currentStep = 0;

    for (auto* series : m_series) {
        delete series;
    }
    m_series.clear();

    for (size_t i = 0; i < numEquations; ++i) {
        m_series.push_back(new QLineSeries(this));
    }

    std::vector<double> y = m_initialConditions;
    double t = m_startTime;
    const double stopThreshold = 1e-09;
    bool stopFlag = false; // Флаг остановки

    // Матрица Якобиана (для вашей системы она постоянна)
    Eigen::Matrix2d A;
    A << -500.005, 499.995,
        499.995, -500.005;

    // Матрица (I - hA)
    Eigen::Matrix2d I = Eigen::Matrix2d::Identity();
    Eigen::Matrix2d M = I - m_stepSize * A;

    // Предварительно инвертируем матрицу, так как система линейная
    Eigen::Matrix2d M_inv = M.inverse();

    while (t <= m_endTime && !stopFlag) {
        // Проверка порогового значения
        bool belowThreshold = std::all_of(y.begin(), y.end(),
                                          [stopThreshold](double val) { return std::abs(val) <= stopThreshold; });

        if (belowThreshold) {
            qDebug() << "Stopped due to value exceeding threshold at t =" << t;
            break;
        }

        if (currentStep++ > maxSteps) {
            qDebug() << "Stopped due to exceeding maximum number of steps.";
            break;
        }

        // Записываем текущие значения в график
        for (size_t i = 0; i < numEquations; ++i) {
            m_series[i]->append(t, y[i]);
        }

        // Вычисляем следующее значение
        double tNext = t + m_stepSize;

        // Решаем линейную систему (I - hA)y_{n+1} = y_n
        Eigen::Vector2d yVec(y[0], y[1]);
        Eigen::Vector2d yNextVec = M_inv * yVec;

        y = { yNextVec[0], yNextVec[1] };
        t = tNext;
    }
}

const std::vector<QLineSeries*>& StiffOdeModel::getSeries() const
{
    return m_series;
}
}
