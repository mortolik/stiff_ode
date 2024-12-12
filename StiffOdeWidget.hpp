#pragma once

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

QT_FORWARD_DECLARE_CLASS(QTableWidget);

using namespace QtCharts;

namespace StiffOde
{
class StiffOdeModel;
class StiffOdeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StiffOdeWidget(StiffOdeModel* model, QWidget* parent = nullptr);

private:
    void setupUi();
    void populateTableAndChart();

    StiffOdeModel* m_model;
    QTableWidget* m_tableWidget;
    QChartView* m_chartView;
    QChart* m_chart;
};
}
