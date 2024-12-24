// StiffOdeWidget.hpp
#ifndef STIFFODEWIDGET_HPP
#define STIFFODEWIDGET_HPP

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QTextEdit>
#include <QTableView>
#include "StiffOdeModel.hpp"
#include "StiffOdeTableModel.hpp"

namespace StiffOde
{
class StiffOdeWidget : public QWidget
{
    Q_OBJECT

public:
    StiffOdeWidget(StiffOdeModel* model, QWidget* parent = nullptr);

private:
    void setupUi();
    void populateTableAndChart();
    void populateExactChart();
    void populateGlobalErrorChart();
    void populateSolutionComparisonChart();
    void populateExactValuesTable();

    StiffOdeModel* m_model;
    QTableView* m_tableView;
    StiffOdeTableModel* m_tableModel;

    QChartView* m_chartView;
    QChart* m_chart;

    QChart* m_exactChart;
    QChartView* m_exactChartView;

    QChart* m_globalErrorChart;
    QChartView* m_globalErrorChartView;

    QWidget* m_solutionComparisonTab;
    QWidget* m_exactValuesTab;

    QTextEdit* m_errorSummaryText;
};
}

#endif // STIFFODEWIDGET_HPP
