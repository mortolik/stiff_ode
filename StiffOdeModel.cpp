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
    while (t <= m_endExactTime)
    {
        Eigen::Vector2d solution = coefficients[0] * exp(eigenValues[0] * t) * eigenVectors.col(0) +
                                   coefficients[1] * exp(eigenValues[1] * t) * eigenVectors.col(1);

        exactSolution.emplace_back(t, solution[0]);
        exactSolution.emplace_back(t, solution[1]);
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

    for (size_t i = 0; i < numSteps; ++i) {
        double t = numericalSolution[0]->at(i).x();

        for (size_t j = 0; j < numComponents; ++j) {
            double numericalValue = numericalSolution[j]->at(i).y();
            double exactValue = exactSolution[i * numComponents + j].y();
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

    for (auto* series : m_series)
    {
        delete series;
    }
    m_series.clear();

    for (size_t i = 0; i < numEquations; ++i)
    {
        m_series.push_back(new QLineSeries(this));
    }

    std::vector<double> y = m_initialConditions;
    double t = m_startTime;

    while (t <= m_endTime)
    {
        for (size_t i = 0; i < numEquations; ++i)
        {
            m_series[i]->append(t, y[i]);
        }

        std::vector<double> yNext = y;
        double tNext = t + m_stepSize;

        for (int iteration = 0; iteration < 100; ++iteration)
        {
            std::vector<double> fValue = m_system(y, t);
            std::vector<double> yTemp = yNext;
            for (size_t i = 0; i < numEquations; ++i)
            {
                yNext[i] = y[i] + m_stepSize * fValue[i];
            }

            double maxDiff = 0.0;
            for (size_t i = 0; i < numEquations; ++i)
            {
                maxDiff = std::max(maxDiff, std::abs(yNext[i] - yTemp[i]));
            }
            if (maxDiff < 1e-6)
                break;
            if (iteration == 99)
            {
                qDebug() << "Warning: Method did not converge at t = " << tNext;
            }
        }

        y = yNext;
        t = tNext;
    }
}


const std::vector<QLineSeries*>& StiffOdeModel::getSeries() const
{
    return m_series;
}
}
