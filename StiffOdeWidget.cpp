#include "StiffOdeModel.hpp"
#include "StiffOdeWidget.hpp"

#include <QDebug>
#include <QTabWidget>
#include <QVBoxLayout>
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
    m_exactChart(new QChart),
    m_globalErrorChart(new QChart)
{
    setupUi();

    populateTableAndChart();
    populateExactChart();
    populateGlobalErrorChart();
    populateSolutionComparisonChart();
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

    QChartView* globalErrorChartView = new QChartView(this);
    globalErrorChartView->setChart(m_globalErrorChart);
    QWidget* errorChartTab = new QWidget(this);
    QVBoxLayout* errorChartLayout = new QVBoxLayout(errorChartTab);
    errorChartLayout->addWidget(globalErrorChartView);
    errorChartTab->setLayout(errorChartLayout);

    tabWidget->addTab(chartTab, "График численного решения");
    tabWidget->addTab(tableTab, "Таблица");
    tabWidget->addTab(exactChartTab, "График точного решения");
    tabWidget->addTab(errorChartTab, "График глобальной погрешности");

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

void StiffOdeWidget::populateGlobalErrorChart()
{
    const auto globalErrors = m_model->computeGlobalError();
    if (globalErrors.empty())
        return;

    if (globalErrors.size() < 2)
    {
        qDebug() << "Недостаточно компонент для построения графиков глобальной погрешности";
        return;
    }

    auto* seriesY0 = new QtCharts::QLineSeries();
    auto* seriesY1 = new QtCharts::QLineSeries();

    seriesY0->setName("E(1) - глобальная погрешность первой компоненты");
    seriesY1->setName("E(2) - глобальная погрешность второй компоненты");

    const std::vector<QColor> colors = {Qt::green, Qt::magenta};
    seriesY0->setColor(colors[0]);
    seriesY1->setColor(colors[1]);

    for (const auto& point : globalErrors[0])
    {
        seriesY0->append(point.x(), point.y());
    }
    for (const auto& point : globalErrors[1])
    {
        seriesY1->append(point.x(), point.y());
    }

    m_globalErrorChart->removeAllSeries();
    m_globalErrorChart->addSeries(seriesY0);
    m_globalErrorChart->addSeries(seriesY1);
    m_globalErrorChart->createDefaultAxes();

    auto* axisY = qobject_cast<QtCharts::QValueAxis*>(m_globalErrorChart->axes(Qt::Vertical).first());
    if (axisY)
    {
        axisY->setLabelFormat("%.6g");
    }

    auto* axisX = qobject_cast<QtCharts::QValueAxis*>(m_globalErrorChart->axes(Qt::Horizontal).first());
    if (axisX)
    {
        axisX->setLabelFormat("%.6g");
    }

    m_globalErrorChart->setTitle("График глобальной погрешности");
}

void StiffOdeWidget::populateSolutionComparisonChart()
{
    const auto& numericalSeries = m_model->getSeries();
    const auto exactSolution = m_model->computeExactSolution();

    if (numericalSeries.empty() || exactSolution.empty())
        return;

    auto* numericalY0 = new QtCharts::QLineSeries();
    auto* numericalY1 = new QtCharts::QLineSeries();
    auto* exactY0 = new QtCharts::QLineSeries();
    auto* exactY1 = new QtCharts::QLineSeries();

    numericalY0->setName("Численное решение v(1)");
    numericalY1->setName("Численное решение v(2)");
    exactY0->setName("Точное решение u(1)");
    exactY1->setName("Точное решение u(2)");

    int numPoints = numericalSeries[0]->count();
    for (int i = 0; i < numPoints; ++i)
    {
        double t = numericalSeries[0]->at(i).x();
        numericalY0->append(t, numericalSeries[0]->at(i).y());
        numericalY1->append(t, numericalSeries[1]->at(i).y());
    }

    for (size_t i = 0; i < exactSolution.size(); i += 2)
    {
        exactY0->append(exactSolution[i].x(), exactSolution[i].y());
        exactY1->append(exactSolution[i + 1].x(), exactSolution[i + 1].y());
    }

    QChart* chartY0 = new QChart();
    chartY0->addSeries(numericalY0);
    chartY0->addSeries(exactY0);
    chartY0->createDefaultAxes();
    chartY0->setTitle("Решение первой компоненты");
    QPen penExact(Qt::blue);
    penExact.setWidth(2);
    exactY0->setPen(penExact);
    QPen penNumerical(Qt::red);
    penNumerical.setWidth(2);
    numericalY0->setPen(penNumerical);

    QChart* chartY1 = new QChart();
    chartY1->addSeries(numericalY1);
    chartY1->addSeries(exactY1);
    chartY1->createDefaultAxes();
    chartY1->setTitle("Решение второй компоненты");
    exactY1->setPen(penExact);
    numericalY1->setPen(penNumerical);

    QVBoxLayout* layout = new QVBoxLayout();
    QChartView* chartViewY0 = new QChartView(chartY0);
    QChartView* chartViewY1 = new QChartView(chartY1);

    layout->addWidget(chartViewY0);
    layout->addWidget(chartViewY1);

    QWidget* comparisonTab = new QWidget();
    comparisonTab->setLayout(layout);
    m_chart->createDefaultAxes();

    auto* axisY = qobject_cast<QtCharts::QValueAxis*>(m_chart->axes(Qt::Vertical).first());
    if (axisY)
    {
        axisY->setLabelFormat("%.6g");
    }

    auto* axisX = qobject_cast<QtCharts::QValueAxis*>(m_chart->axes(Qt::Horizontal).first());
    if (axisX)
    {
        axisX->setLabelFormat("%.6g");
    }

    m_chart->setTitle("Решение жёсткой системы ОДУ");

    QTabWidget* tabWidget = findChild<QTabWidget*>();
    if (tabWidget)
    {
        tabWidget->addTab(comparisonTab, "Сравнение решений");
    }
}

}
