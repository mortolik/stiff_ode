#include "StiffOdeModel.hpp"
#include "StiffOdeWidget.hpp"

#include <QVBoxLayout>
#include <QTabWidget>
#include <QTableWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

namespace StiffOde
{
StiffOdeWidget::StiffOdeWidget(StiffOdeModel* model, QWidget* parent)
    : QWidget(parent), m_model(model), m_tableWidget(new QTableWidget(this)), m_chartView(new QChartView(this)), m_chart(new QChart)
{
    setupUi();

    populateTableAndChart();
}

void StiffOdeWidget::setupUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    // Создаем табы
    QTabWidget* tabWidget = new QTabWidget(this);

    // Настраиваем график
    m_chartView->setChart(m_chart);
    QWidget* chartTab = new QWidget(this);
    QVBoxLayout* chartLayout = new QVBoxLayout(chartTab);
    chartLayout->addWidget(m_chartView);
    chartTab->setLayout(chartLayout);

    // Настраиваем таблицу
    QWidget* tableTab = new QWidget(this);
    QVBoxLayout* tableLayout = new QVBoxLayout(tableTab);
    tableLayout->addWidget(m_tableWidget);
    tableTab->setLayout(tableLayout);

    // Добавляем вкладки
    tabWidget->addTab(chartTab, "График");
    tabWidget->addTab(tableTab, "Таблица");

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

    // Заполняем таблицу
    m_tableWidget->setRowCount(numPoints);
    m_tableWidget->setColumnCount(numVariables + 1); // Время + переменные

    QStringList headers;
    headers << "Time";
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

    // Добавляем серии в график
    m_chart->removeAllSeries(); // Удаляем старые серии, чтобы избежать дублирования
    for (int j = 0; j < numVariables; ++j) {
        auto series = new QtCharts::QLineSeries();
        series->setName(QString("u(%1)").arg(j + 1));

        for (int i = 0; i < numPoints; ++i) {
            double xValue = seriesList[0]->at(i).x();
            double yValue = seriesList[j]->at(i).y();
            series->append(xValue, yValue);
        }

        m_chart->addSeries(series);
    }

    // Настраиваем график
    m_chart->createDefaultAxes();
    m_chart->setTitle("Solution of the Stiff ODE System");
}

}
