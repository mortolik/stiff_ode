#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineSeries>
#include <QDoubleSpinBox>
#include <QtCharts/QChartView>

#include "mainwindow.h"
#include "StiffOdeModel.hpp"
#include "StiffOdeWidget.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setObjectName("mainWindow");
    setMinimumSize(800, 600);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setAlignment(Qt::AlignTop);

    QHBoxLayout *inputLayout = new QHBoxLayout();

    QLabel *stepSizeLabel = new QLabel("Step size (h):", this);
    QDoubleSpinBox *stepSizeSpinBox = new QDoubleSpinBox(this);
    stepSizeSpinBox->setRange(0.001, 1.0);
    stepSizeSpinBox->setDecimals(3);
    stepSizeSpinBox->setValue(0.001);
    stepSizeSpinBox->setSingleStep(0.001);
    inputLayout->addWidget(stepSizeLabel);
    inputLayout->addWidget(stepSizeSpinBox);

    QLabel *startTimeLabel = new QLabel("Start time (t0):", this);
    QDoubleSpinBox *startTimeSpinBox = new QDoubleSpinBox(this);
    startTimeSpinBox->setRange(0, 10000);
    startTimeSpinBox->setDecimals(3);
    startTimeSpinBox->setValue(0);
    inputLayout->addWidget(startTimeLabel);
    inputLayout->addWidget(startTimeSpinBox);

    QLabel *endTimeLabel = new QLabel("End time (tN):", this);
    QDoubleSpinBox *endTimeSpinBox = new QDoubleSpinBox(this);
    endTimeSpinBox->setRange(0, 10000);
    endTimeSpinBox->setDecimals(3);
    endTimeSpinBox->setValue(1.0);
    inputLayout->addWidget(endTimeLabel);
    inputLayout->addWidget(endTimeSpinBox);

    mainLayout->addLayout(inputLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *createModelButton = new QPushButton("Create Model", this);
    buttonLayout->addWidget(createModelButton);

    mainLayout->addLayout(buttonLayout);

    connect(createModelButton, &QPushButton::clicked, this, [=]() {
        if (m_model != nullptr)
            delete m_model;
        if (m_widget != nullptr) {
            centralWidget->layout()->removeWidget(m_widget);
            delete m_widget;
        }
        double stepSize = stepSizeSpinBox->value();
        double startTime = startTimeSpinBox->value();
        double endTime = endTimeSpinBox->value();

        m_model= new StiffOde::StiffOdeModel(this);
        m_model->setInitialConditions({7, 13}, startTime);
        m_model->setParameters(stepSize, endTime);
        m_model->solve();

        m_widget = new StiffOde::StiffOdeWidget(m_model, this);

        centralWidget->layout()->addWidget(m_widget);
    });

}


MainWindow::~MainWindow()
{
    delete m_model;
    delete m_widget;
}
