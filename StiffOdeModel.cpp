#include "StiffOdeModel.hpp"
#include <QObject>
#include <QtCharts/QLineSeries>
#include <vector>
#include <functional>

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

void StiffOdeModel::setParameters(double stepSize, double endTime)
{
    m_stepSize = stepSize;
    m_endTime = endTime;
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

        for (int iteration = 0; iteration < 10; ++iteration)
        {
            std::vector<double> fValue = m_system(yNext, tNext);
            for (size_t i = 0; i < numEquations; ++i)
            {
                yNext[i] = y[i] + m_stepSize * fValue[i];
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
