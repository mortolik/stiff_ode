#pragma once

#include <QObject>
#include <QtCharts/QLineSeries>

using namespace QtCharts;

namespace StiffOde
{
class StiffOdeModel : public QObject
{
    Q_OBJECT

public:
    explicit StiffOdeModel(QObject* parent = nullptr);
    void setSystem(const std::function<std::vector<double>(const std::vector<double>&, double)>& system);
    void setInitialConditions(const std::vector<double>& initialConditions, double startTime);
    void setParameters(double stepSize, double endTime, double endExactTime);
    void solve();
    const std::vector<QLineSeries*>& getSeries() const;
    std::vector<QPointF> computeExactSolution() const;
    double getExactEndTime();

private:
    std::function<std::vector<double>(const std::vector<double>&, double)> m_system;
    std::vector<double> m_initialConditions;
    double m_startTime;
    double m_endTime;
    double m_endExactTime;
    double m_stepSize;
    std::vector<QLineSeries*> m_series;
};
}
