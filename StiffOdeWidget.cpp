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
    m_tableView(new QTableView(this)),
    m_tableModel(new StiffOdeTableModel(this)),
    m_chartView(new QChartView(this)),
    m_chart(new QChart),
    m_exactChart(new QChart),
    m_globalErrorChart(new QChart),
    m_solutionComparisonTab(new QWidget(this)),
    m_exactValuesTab(new QWidget(this)),
    m_errorSummaryText(new QTextEdit(this))
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

    // Основной график
    m_chartView->setChart(m_chart);
    QWidget* chartTab = new QWidget(this);
    QVBoxLayout* chartLayout = new QVBoxLayout(chartTab);
    chartLayout->addWidget(m_chartView);
    chartTab->setLayout(chartLayout);

    // Таблица и справка
    QWidget* tableTab = new QWidget(this);
    QVBoxLayout* tableLayout = new QVBoxLayout(tableTab);
    tableLayout->addWidget(m_tableView);

    m_errorSummaryText->setReadOnly(true);
    m_errorSummaryText->setMaximumHeight(150);
    tableLayout->addWidget(m_errorSummaryText);

    tableTab->setLayout(tableLayout);

    // График точного решения
    m_exactChartView = new QChartView(this);
    m_exactChartView->setChart(m_exactChart);
    QWidget* exactChartTab = new QWidget(this);
    QVBoxLayout* exactChartLayout = new QVBoxLayout(exactChartTab);
    exactChartLayout->addWidget(m_exactChartView);
    exactChartTab->setLayout(exactChartLayout);

    // График глобальной погрешности
    m_globalErrorChartView = new QChartView(this);
    m_globalErrorChartView->setChart(m_globalErrorChart);
    QWidget* errorChartTab = new QWidget(this);
    QVBoxLayout* errorChartLayout = new QVBoxLayout(errorChartTab);
    errorChartLayout->addWidget(m_globalErrorChartView);
    errorChartTab->setLayout(errorChartLayout);

    // Сравнение решений
    QWidget* solutionComparisonTab = m_solutionComparisonTab;
    QVBoxLayout* comparisonLayout = new QVBoxLayout(solutionComparisonTab);
    solutionComparisonTab->setLayout(comparisonLayout);

    // Точные значения
    QWidget* exactValuesTab = m_exactValuesTab;
    QVBoxLayout* exactValuesLayout = new QVBoxLayout(exactValuesTab);
    exactValuesTab->setLayout(exactValuesLayout);

    tabWidget->addTab(chartTab, "График численного решения");
    tabWidget->addTab(tableTab, "Таблица и справка");
    tabWidget->addTab(exactChartTab, "График точного решения");
    tabWidget->addTab(errorChartTab, "График глобальной погрешности");
    tabWidget->addTab(solutionComparisonTab, "Сравнение решений");
    tabWidget->addTab(exactValuesTab, "Точные значения");

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

    size_t numSteps = seriesList[0]->count();
    size_t numVariables = seriesList.size();

    // Проверка соответствия размеров
    size_t maxAllowedSteps = exactSolution.size() / numVariables;
    if (numSteps > maxAllowedSteps) {
        qDebug() << "Численное решение превышает количество точек точного решения.";
        numSteps = maxAllowedSteps; // Ограничиваем количество шагов
    }

    // Извлекаем данные для модели таблицы
    std::vector<QPointF> numericalSeries0;
    std::vector<QPointF> numericalSeries1;
    std::vector<QPointF> exactSeries0;
    std::vector<QPointF> exactSeries1;
    std::vector<QPointF> error0;
    std::vector<QPointF> error1;

    for (size_t i = 0; i < numSteps; ++i) {
        numericalSeries0.push_back(seriesList[0]->at(i));
        if (numVariables > 1)
            numericalSeries1.push_back(seriesList[1]->at(i));

        exactSeries0.emplace_back(exactSolution[i * numVariables].x(), exactSolution[i * numVariables].y());
        if (numVariables > 1)
            exactSeries1.emplace_back(exactSolution[i * numVariables + 1].x(), exactSolution[i * numVariables + 1].y());

        if (globalErrors.size() > 0)
            error0.emplace_back(globalErrors[0][i].x(), globalErrors[0][i].y());
        if (globalErrors.size() > 1)
            error1.emplace_back(globalErrors[1][i].x(), globalErrors[1][i].y());
    }

    // Устанавливаем данные в модель таблицы
    m_tableModel->setData(numericalSeries0, numericalSeries1, exactSeries0, exactSeries1, error0, error1);
    m_tableView->setModel(m_tableModel);

    // Настройка таблицы для лучшей производительности
    m_tableView->setSortingEnabled(true);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Теперь строим график с пропуском точек
    m_chart->removeAllSeries();
    for (size_t j = 0; j < numVariables; ++j)
    {
        auto chartSeries = new QtCharts::QLineSeries();
        chartSeries->setName(QString("u(%1)").arg(j + 1));

        // Пропуск для графика
        const int MAX_POINTS = 10000;
        int skipFactor = (numSteps > MAX_POINTS)
                             ? static_cast<int>(std::ceil(double(numSteps) / MAX_POINTS))
                             : 1;

        // Заполняем новую серию с учётом skipFactor
        for (size_t i = 0; i < numSteps; i += skipFactor)
        {
            double xValue = seriesList[j]->at(i).x();
            double yValue = seriesList[j]->at(i).y();
            chartSeries->append(xValue, yValue);
        }

        m_chart->addSeries(chartSeries);
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

    size_t numVariables = 2; // Предполагаем, что 2 переменные
    size_t totalPairs = exactSolution.size() / numVariables;
    auto* seriesY0 = new QtCharts::QLineSeries();
    auto* seriesY1 = new QtCharts::QLineSeries();

    seriesY0->setName("Точное решение u(1)");
    seriesY1->setName("Точное решение u(2)");

    const int MAX_POINTS = 10000;
    int skipFactor = (totalPairs > MAX_POINTS)
                         ? static_cast<int>(std::ceil(double(totalPairs) / MAX_POINTS))
                         : 1;

    for (size_t i = 0; i < totalPairs; i += skipFactor)
    {
        // i-я пара: [2*i] = y0, [2*i+1] = y1
        const auto& pointY0 = exactSolution[2 * i];
        const auto& pointY1 = exactSolution[2 * i + 1];

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
        m_errorSummaryText->setText("Недостаточно компонент для построения графиков глобальной погрешности.");
        return;
    }

    size_t numSteps = globalErrors[0].size();
    auto* seriesY0 = new QtCharts::QLineSeries();
    auto* seriesY1 = new QtCharts::QLineSeries();

    seriesY0->setName("E(1) - глобальная погрешность первой компоненты");
    seriesY1->setName("E(2) - глобальная погрешность второй компоненты");

    const int MAX_POINTS = 10000;
    int skipFactor = (numSteps > MAX_POINTS)
                         ? static_cast<int>(std::ceil(double(numSteps) / MAX_POINTS))
                         : 1;

    double maxErrorY0 = std::numeric_limits<double>::lowest();
    double maxErrorStepY0 = 0.0;

    double maxErrorY1 = std::numeric_limits<double>::lowest();
    double maxErrorStepY1 = 0.0;

    for (size_t i = 0; i < numSteps; i += skipFactor)
    {
        const auto& point0 = globalErrors[0][i];
        const auto& point1 = globalErrors[1][i];

        seriesY0->append(point0.x(), point0.y());
        seriesY1->append(point1.x(), point1.y());

        // Обновление максимальных значений
        if (point0.y() > maxErrorY0) {
            maxErrorY0 = point0.y();
            maxErrorStepY0 = point0.x();
        }

        if (point1.y() > maxErrorY1) {
            maxErrorY1 = point1.y();
            maxErrorStepY1 = point1.x();
        }
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

    // Получение последних численных решений
    const auto& series = m_model->getSeries();
    QString lastSolutionsText;
    if (!series.empty())
    {
        double lastX = series[0]->at(series[0]->count() - 1).x();
        lastSolutionsText += QString("\nПоследний x: %1").arg(lastX);

        for (size_t i = 0; i < series.size(); ++i)
        {
            double lastY = series[i]->at(series[i]->count() - 1).y();
            lastSolutionsText += QString("\nПоследнее значение компоненты %1: %2").arg(i + 1).arg(lastY);
        }
    }

    QString summaryText;

    summaryText += QString("Задача была решена с шагом: %1 \n").arg(m_model->getStepSize());
    summaryText += QString("\nПервая компонента:\n");
    summaryText += QString("  Максимальная погрешность: %1 в точке x = %2\n").arg(maxErrorY0).arg(maxErrorStepY0);

    summaryText += QString("\nВторая компонента:\n");
    summaryText += QString("  Максимальная погрешность: %1 в точке x = %2\n").arg(maxErrorY1).arg(maxErrorStepY1);
    summaryText += QString("\nКоличество шагов: %1 \n").arg(numSteps);
    summaryText += lastSolutionsText;

    m_errorSummaryText->setText(summaryText);
}

void StiffOdeWidget::populateSolutionComparisonChart()
{
    const auto& numericalSeries = m_model->getSeries();
    const auto exactSolution = m_model->computeExactSolution();

    if (numericalSeries.empty() || exactSolution.empty())
        return;

    size_t numSteps = numericalSeries[0]->count();
    size_t numVariables = numericalSeries.size();

    // Предполагаем, что numVariables = 2
    auto* numericalY0 = new QtCharts::QLineSeries();
    auto* numericalY1 = new QtCharts::QLineSeries();
    auto* exactY0 = new QtCharts::QLineSeries();
    auto* exactY1 = new QtCharts::QLineSeries();

    numericalY0->setName("Численное решение v(1)");
    numericalY1->setName("Численное решение v(2)");
    exactY0->setName("Точное решение u(1)");
    exactY1->setName("Точное решение u(2)");

    const int MAX_POINTS = 10000;
    int skipFactor = (numSteps > MAX_POINTS)
                         ? static_cast<int>(std::ceil(double(numSteps) / MAX_POINTS))
                         : 1;

    for (size_t i = 0; i < numSteps; i += skipFactor)
    {
        double t = numericalSeries[0]->at(i).x();
        numericalY0->append(t, numericalSeries[0]->at(i).y());
        numericalY1->append(t, numericalSeries[1]->at(i).y());

        exactY0->append(exactSolution[i * numVariables].x(), exactSolution[i * numVariables].y());
        exactY1->append(exactSolution[i * numVariables + 1].x(), exactSolution[i * numVariables + 1].y());
    }

    QChart* chartY0 = new QChart();
    chartY0->addSeries(numericalY0);
    chartY0->addSeries(exactY0);

    auto* axisX0 = new QtCharts::QValueAxis();
    axisX0->setLabelFormat("%.6g");
    axisX0->setTitleText("Значения x");
    chartY0->addAxis(axisX0, Qt::AlignBottom);
    numericalY0->attachAxis(axisX0);
    exactY0->attachAxis(axisX0);

    auto* axisY0 = new QtCharts::QValueAxis();
    axisY0->setLabelFormat("%.6g");
    axisY0->setTitleText("Значения u(1) и v(1)");
    chartY0->addAxis(axisY0, Qt::AlignLeft);
    numericalY0->attachAxis(axisY0);
    exactY0->attachAxis(axisY0);

    chartY0->setTitle("Решение первой компоненты");

    QChart* chartY1 = new QChart();
    chartY1->addSeries(numericalY1);
    chartY1->addSeries(exactY1);

    auto* axisX1 = new QtCharts::QValueAxis();
    axisX1->setLabelFormat("%.6g");
    axisX1->setTitleText("Значения x");
    chartY1->addAxis(axisX1, Qt::AlignBottom);
    numericalY1->attachAxis(axisX1);
    exactY1->attachAxis(axisX1);

    auto* axisY1 = new QtCharts::QValueAxis();
    axisY1->setLabelFormat("%.6g");
    axisY1->setTitleText("Значения u(2) и v(2)");
    chartY1->addAxis(axisY1, Qt::AlignLeft);
    numericalY1->attachAxis(axisY1);
    exactY1->attachAxis(axisY1);

    chartY1->setTitle("Решение второй компоненты");

    auto* layout = qobject_cast<QVBoxLayout*>(m_solutionComparisonTab->layout());
    if (layout)
    {
        // Очистка предыдущих графиков
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
    exactValuesTable->setColumnCount(6);

    QStringList headers;
    headers << "n" << "x_n" << "exp(-0.01 * x)" << "exp(-1000 * x)" << "u(1) (точное)" << "u(2) (точное)";
    exactValuesTable->setHorizontalHeaderLabels(headers);
    exactValuesTable->verticalHeader()->setVisible(false);

    for (size_t i = 0; i < exactSolution.size() / 2; ++i)
    {
        double t = exactSolution[i * 2].x();
        double u1 = exactSolution[i * 2].y();
        double u2 = exactSolution[i * 2 + 1].y();

        QTableWidgetItem* rowIndexItem = new QTableWidgetItem(QString::number(i));
        exactValuesTable->setItem(i, 0, rowIndexItem);

        QTableWidgetItem* timeItem = new QTableWidgetItem(QString::number(t, 'f', 16));
        exactValuesTable->setItem(i, 1, timeItem);

        QTableWidgetItem* expSmallItem = new QTableWidgetItem(QString::number(std::exp(-0.01 * t), 'f', 16));
        exactValuesTable->setItem(i, 2, expSmallItem);

        QTableWidgetItem* expLargeItem = new QTableWidgetItem(QString::number(std::exp(-1000 * t), 'e', 16));
        exactValuesTable->setItem(i, 3, expLargeItem);

        QTableWidgetItem* exactU1Item = new QTableWidgetItem(QString::number(u1, 'f', 16));
        exactValuesTable->setItem(i, 4, exactU1Item);

        QTableWidgetItem* exactU2Item = new QTableWidgetItem(QString::number(u2, 'f', 16));
        exactValuesTable->setItem(i, 5, exactU2Item);
    }

    exactValuesTable->horizontalHeader()->setMinimumSectionSize(25 * QFontMetrics(exactValuesTable->font()).horizontalAdvance('0'));

    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(m_exactValuesTab->layout());
    if (layout)
    {
        // Очистка предыдущих таблиц
        while (QLayoutItem* item = layout->takeAt(0))
        {
            delete item->widget();
            delete item;
        }
        layout->addWidget(exactValuesTable);
    }
}
}
