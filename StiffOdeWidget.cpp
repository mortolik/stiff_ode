#include "StiffOdeModel.hpp"
#include "StiffOdeWidget.hpp"

#include <QVBoxLayout>
#include <QTabWidget>
#include <QTableWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>

namespace StiffOde
{
StiffOdeWidget::StiffOdeWidget(StiffOdeModel* model, QWidget* parent)
    : QWidget(parent),
    m_model(model),
    m_tableWidget(new QTableWidget(this)),
    m_chartView(new QChartView(this)),
    m_chart(new QChart),
    m_exactChart(new QChart)
{
    setupUi();

    populateTableAndChart();
    populateExactChart();
}

void StiffOdeWidget::setupUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QTabWidget* tabWidget = new QTabWidget(this);

    m_chartView->setChart(m_chart);
    QWidget* chartTab = new QWidget(this);
    QVBoxLayout* chartLayout = new QVBoxLayout(chartTab);
    chartLayout->addWidget(m_chartView);
    chartTab->setLayout(chartLayout);

    QWidget* tableTab = new QWidget(this);
    QVBoxLayout* tableLayout = new QVBoxLayout(tableTab);
    tableLayout->addWidget(m_tableWidget);
    tableTab->setLayout(tableLayout);

    QChartView* exactChartView = new QChartView(this);
    exactChartView->setChart(m_exactChart);
    QWidget* exactChartTab = new QWidget(this);
    QVBoxLayout* exactChartLayout = new QVBoxLayout(exactChartTab);
    exactChartLayout->addWidget(exactChartView);
    exactChartTab->setLayout(exactChartLayout);

    tabWidget->addTab(chartTab, "График численного решения");
    tabWidget->addTab(tableTab, "Таблица");
    tabWidget->addTab(exactChartTab, "График точного решения");

    layout->addWidget(tabWidget);
    setLayout(layout);
}
void StiffOdeWidget::populateTableAndChart()
{
    const auto& seriesList = m_model->getSeries();
    if (seriesList.empty())
        return;

    int numPoints = seriesList[0]->count();
    int numVariables = static_cast<int>(seriesList.size());

    m_tableWidget->setRowCount(numPoints);
    m_tableWidget->setColumnCount(numVariables + 1);

    QStringList headers;
    headers << "Время";
    for (int i = 0; i < numVariables; ++i) {
        headers << QString("u(%1)").arg(i + 1);
    }
    m_tableWidget->setHorizontalHeaderLabels(headers);

    for (int i = 0; i < numPoints; ++i) {
        QTableWidgetItem* timeItem = new QTableWidgetItem(QString::number(seriesList[0]->at(i).x(), 'f', 6));
        m_tableWidget->setItem(i, 0, timeItem);

        for (int j = 0; j < numVariables; ++j) {
            double yValue = seriesList[j]->at(i).y();
            QTableWidgetItem* valueItem = new QTableWidgetItem(QString::number(yValue, 'f', 6));
            m_tableWidget->setItem(i, j + 1, valueItem);
        }
    }

    m_chart->removeAllSeries();
    for (int j = 0; j < numVariables; ++j) {
        auto series = new QtCharts::QLineSeries();
        series->setName(QString("v(%1)").arg(j + 1));

        for (int i = 0; i < numPoints; ++i) {
            double xValue = seriesList[0]->at(i).x();
            double yValue = seriesList[j]->at(i).y();
            series->append(xValue, yValue);
        }

        m_chart->addSeries(series);
    }

    m_chart->createDefaultAxes();
    auto* axisY = qobject_cast<QtCharts::QValueAxis*>(m_chart->axes(Qt::Vertical).first());
    if (axisY) {
        axisY->setLabelFormat("%.6g");
    }

    auto* axisX = qobject_cast<QtCharts::QValueAxis*>(m_chart->axes(Qt::Horizontal).first());
    if (axisX) {
        axisX->setLabelFormat("%.6g");
    }
    m_chart->setTitle("Решение жёсткой системы ОДУ");
}

void StiffOdeWidget::populateExactChart()
{

    const auto exactSolution = m_model->computeExactSolution();
    if (exactSolution.empty())
        return;

    auto* seriesY0 = new QtCharts::QLineSeries();
    auto* seriesY1 = new QtCharts::QLineSeries();

    seriesY0->setName("Точное решение u(1)");
    seriesY1->setName("Точное решение u(2)");

    int SKIP_POINTS = 1;

    if (m_model->getExactEndTime() > 999)
    {
        SKIP_POINTS = 10;
    }
    else
    {
        SKIP_POINTS = 1;
    }

    for (size_t i = 0; i < exactSolution.size(); i += 2 * SKIP_POINTS)
    {
        const auto& pointY0 = exactSolution[i];
        const auto& pointY1 = exactSolution[i + 1];

        seriesY0->append(pointY0.x(), pointY0.y());
        seriesY1->append(pointY1.x(), pointY1.y());
    }

    m_exactChart->removeAllSeries();
    m_exactChart->addSeries(seriesY0);
    m_exactChart->addSeries(seriesY1);

    m_exactChart->createDefaultAxes();

    auto* axisY = qobject_cast<QtCharts::QValueAxis*>(m_exactChart->axes(Qt::Vertical).first());
    if (axisY) {
        axisY->setLabelFormat("%.6g");
    }

    auto* axisX = qobject_cast<QtCharts::QValueAxis*>(m_exactChart->axes(Qt::Horizontal).first());
    if (axisX) {
        axisX->setLabelFormat("%.6g");
    }

    m_exactChart->setTitle("Точное решение жёсткой системы ОДУ");

    QPen penY0(Qt::blue);
    penY0.setWidth(2);
    seriesY0->setPen(penY0);

    QPen penY1(Qt::red);
    penY1.setWidth(2);
    seriesY1->setPen(penY1);
}

}
