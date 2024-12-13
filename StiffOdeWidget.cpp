#include "StiffOdeModel.hpp"
#include "StiffOdeWidget.hpp"

#include <QDebug>
#include <QTabWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>
#include <cmath>

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
    populateExactValuesTable();
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

    m_solutionComparisonTab = new QWidget(this);
    QVBoxLayout* comparisonLayout = new QVBoxLayout(m_solutionComparisonTab);
    m_solutionComparisonTab->setLayout(comparisonLayout);

    m_exactValuesTab = new QWidget(this);
    QVBoxLayout* exactValuesLayout = new QVBoxLayout(m_exactValuesTab);
    m_exactValuesTab->setLayout(exactValuesLayout);

    tabWidget->addTab(chartTab, "График численного решения");
    tabWidget->addTab(tableTab, "Таблица");
    tabWidget->addTab(exactChartTab, "График точного решения");
    tabWidget->addTab(errorChartTab, "График глобальной погрешности");
    tabWidget->addTab(m_solutionComparisonTab, "Сравнение решений");
    tabWidget->addTab(m_exactValuesTab, "Точные значения");


    layout->addWidget(tabWidget);
    setLayout(layout);
}

void StiffOdeWidget::populateTableAndChart()
{
    const auto& seriesList = m_model->getSeries();
    const auto exactSolution = m_model->computeExactSolution();
    const auto globalErrors = m_model->computeGlobalError();

    if (seriesList.empty() || exactSolution.empty() || globalErrors.empty())
        return;

    int numPoints = seriesList[0]->count();
    int numVariables = static_cast<int>(seriesList.size());

    m_tableWidget->setRowCount(numPoints);
    m_tableWidget->setColumnCount(1 + numVariables * 3);

    for (int col = 0; col < m_tableWidget->columnCount(); ++col)
    {
        m_tableWidget->setColumnWidth(col, 25 * QFontMetrics(m_tableWidget->font()).horizontalAdvance('0'));
    }

    QStringList headers;
    headers << "Время";
    for (int i = 0; i < numVariables; ++i)
    {
        headers << QString("u(%1) (точное)").arg(i + 1);
    }
    for (int i = 0; i < numVariables; ++i)
    {
        headers << QString("u(%1) (численное)").arg(i + 1);
    }
    for (int i = 0; i < numVariables; ++i)
    {
        headers << QString("E(%1) (погрешность)").arg(i + 1);
    }
    m_tableWidget->setHorizontalHeaderLabels(headers);

    for (int i = 0; i < numPoints; ++i)
    {
        QTableWidgetItem* timeItem = new QTableWidgetItem(QString::number(seriesList[0]->at(i).x(), 'g', 16));
        m_tableWidget->setItem(i, 0, timeItem);

        for (int j = 0; j < numVariables; ++j)
        {
            double exactValue = exactSolution[i * numVariables + j].y();
            QTableWidgetItem* exactItem = new QTableWidgetItem(QString::number(exactValue, 'g', 16));
            m_tableWidget->setItem(i, 1 + j, exactItem);
        }

        for (int j = 0; j < numVariables; ++j)
        {
            double numericalValue = seriesList[j]->at(i).y();
            QTableWidgetItem* numericalItem = new QTableWidgetItem(QString::number(numericalValue, 'g', 16));
            m_tableWidget->setItem(i, 1 + numVariables + j, numericalItem);
        }

        for (int j = 0; j < numVariables; ++j)
        {
            double errorValue = globalErrors[j][i].y();
            QTableWidgetItem* errorItem = new QTableWidgetItem(QString::number(errorValue, 'g', 16));
            m_tableWidget->setItem(i, 1 + 2 * numVariables + j, errorItem);
        }
    }

    m_chart->removeAllSeries();
    for (int j = 0; j < numVariables; ++j)
    {
        auto series = new QtCharts::QLineSeries();
        series->setName(QString("u(%1)").arg(j + 1));

        for (int i = 0; i < numPoints; ++i)
        {
            double xValue = seriesList[0]->at(i).x();
            double yValue = seriesList[j]->at(i).y();
            series->append(xValue, yValue);
        }

        m_chart->addSeries(series);
    }

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

    auto* axisX0 = new QtCharts::QValueAxis();
    axisX0->setLabelFormat("%.6g");
    axisX0->setTitleText("Время");
    chartY0->addAxis(axisX0, Qt::AlignBottom);
    numericalY0->attachAxis(axisX0);
    exactY0->attachAxis(axisX0);

    auto* axisY0 = new QtCharts::QValueAxis();
    axisY0->setLabelFormat("%.6g");
    axisY0->setTitleText("Значение");
    chartY0->addAxis(axisY0, Qt::AlignLeft);
    numericalY0->attachAxis(axisY0);
    exactY0->attachAxis(axisY0);

    chartY0->setTitle("Решение первой компоненты");

    QChart* chartY1 = new QChart();
    chartY1->addSeries(numericalY1);
    chartY1->addSeries(exactY1);

    auto* axisX1 = new QtCharts::QValueAxis();
    axisX1->setLabelFormat("%.6g");
    axisX1->setTitleText("Время");
    chartY1->addAxis(axisX1, Qt::AlignBottom);
    numericalY1->attachAxis(axisX1);
    exactY1->attachAxis(axisX1);

    auto* axisY1 = new QtCharts::QValueAxis();
    axisY1->setLabelFormat("%.6g");
    axisY1->setTitleText("Значение");
    chartY1->addAxis(axisY1, Qt::AlignLeft);
    numericalY1->attachAxis(axisY1);
    exactY1->attachAxis(axisY1);

    chartY1->setTitle("Решение второй компоненты");

    auto* layout = qobject_cast<QVBoxLayout*>(m_solutionComparisonTab->layout());
    if (layout)
    {
        while (QLayoutItem* item = layout->takeAt(0))
        {
            delete item->widget();
            delete item;
        }

        QChartView* chartViewY0 = new QChartView(chartY0);
        QChartView* chartViewY1 = new QChartView(chartY1);

        layout->addWidget(chartViewY0);
        layout->addWidget(chartViewY1);
    }
}

void StiffOdeWidget::populateExactValuesTable()
{
    const auto exactSolution = m_model->computeExactSolution();
    if (exactSolution.empty())
        return;

    QTableWidget* exactValuesTable = new QTableWidget(this);
    exactValuesTable->setRowCount(exactSolution.size() / 2);
    exactValuesTable->setColumnCount(4);

    QStringList headers;
    headers << "Время" << "exp(-0.01 * x)" << "exp(-1000 * x)" << "u(1) (точное)" << "u(2) (точное)";
    exactValuesTable->setHorizontalHeaderLabels(headers);

    for (size_t i = 0; i < exactSolution.size() / 2; ++i)
    {
        double t = exactSolution[i * 2].x();
        double u1 = exactSolution[i * 2].y();
        double u2 = exactSolution[i * 2 + 1].y();

        QTableWidgetItem* timeItem = new QTableWidgetItem(QString::number(t, 'f', 16));
        exactValuesTable->setItem(i, 0, timeItem);

        QTableWidgetItem* expSmallItem = new QTableWidgetItem(QString::number(exp(-0.01 * t), 'f', 16));
        exactValuesTable->setItem(i, 1, expSmallItem);

        QTableWidgetItem* expLargeItem = new QTableWidgetItem(QString::number(exp(-1000 * t), 'e', 16));
        exactValuesTable->setItem(i, 2, expLargeItem);

        QTableWidgetItem* exactU1Item = new QTableWidgetItem(QString::number(u1, 'f', 16));
        exactValuesTable->setItem(i, 3, exactU1Item);

        QTableWidgetItem* exactU2Item = new QTableWidgetItem(QString::number(u2, 'f', 16));
        exactValuesTable->setItem(i, 4, exactU2Item);
    }

    exactValuesTable->horizontalHeader()->setMinimumSectionSize(20 * QFontMetrics(exactValuesTable->font()).horizontalAdvance('0'));
    exactValuesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(m_exactValuesTab->layout());
    if (layout)
    {
        while (QLayoutItem* item = layout->takeAt(0))
        {
            delete item->widget();
            delete item;
        }
        layout->addWidget(exactValuesTable);
    }
}

}
