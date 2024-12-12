#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
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
    mainLayout->setSpacing(10);

    QHBoxLayout *groupBoxesLayout = new QHBoxLayout();
    groupBoxesLayout->setSpacing(10);
    groupBoxesLayout->setAlignment(Qt::AlignLeft);

    QGroupBox *inputGroupBox1 = new QGroupBox("Основные параметры", this);
    QVBoxLayout *groupBoxLayout1 = new QVBoxLayout(inputGroupBox1);
    groupBoxLayout1->setSpacing(10);

    QHBoxLayout *inputLayout1 = new QHBoxLayout();
    inputLayout1->setSpacing(10);

    QLabel *stepSizeLabel = new QLabel("Размер шага:", this);
    QDoubleSpinBox *stepSizeSpinBox = new QDoubleSpinBox(this);
    stepSizeSpinBox->setRange(0.001, 1.0);
    stepSizeSpinBox->setDecimals(3);
    stepSizeSpinBox->setValue(0.001);
    stepSizeSpinBox->setSingleStep(0.001);
    inputLayout1->addWidget(stepSizeLabel);
    inputLayout1->addWidget(stepSizeSpinBox);

    QLabel *startTimeLabel = new QLabel("Начальное время:", this);
    QDoubleSpinBox *startTimeSpinBox = new QDoubleSpinBox(this);
    startTimeSpinBox->setRange(0, 10000);
    startTimeSpinBox->setDecimals(3);
    startTimeSpinBox->setValue(0);
    inputLayout1->addWidget(startTimeLabel);
    inputLayout1->addWidget(startTimeSpinBox);

    QLabel *endTimeLabel = new QLabel("Конечное время:", this);
    QDoubleSpinBox *endTimeSpinBox = new QDoubleSpinBox(this);
    endTimeSpinBox->setRange(0, 10000);
    endTimeSpinBox->setDecimals(3);
    endTimeSpinBox->setValue(1.0);
    inputLayout1->addWidget(endTimeLabel);
    inputLayout1->addWidget(endTimeSpinBox);

    groupBoxLayout1->addLayout(inputLayout1);
    groupBoxesLayout->addWidget(inputGroupBox1);

    QGroupBox *inputGroupBox2 = new QGroupBox("Параметры точного решения", this);
    QVBoxLayout *groupBoxLayout2 = new QVBoxLayout(inputGroupBox2);
    groupBoxLayout2->setSpacing(10);

    QHBoxLayout *inputLayout2 = new QHBoxLayout();
    inputLayout2->setSpacing(10);

    QLabel *startExactTimeLabel = new QLabel("Начальное время:", this);
    QDoubleSpinBox *startExactTimeSpinBox = new QDoubleSpinBox(this);
    startExactTimeSpinBox->setRange(0, 10000);
    startExactTimeSpinBox->setDecimals(3);
    startExactTimeSpinBox->setValue(0);
    inputLayout2->addWidget(startExactTimeLabel);
    inputLayout2->addWidget(startExactTimeSpinBox);
    QLabel *endExactTimeLabel = new QLabel("Конечное время:", this);
    QDoubleSpinBox *endExactTimeSpinBox = new QDoubleSpinBox(this);
    endExactTimeSpinBox->setRange(0, 10000);
    endExactTimeSpinBox->setDecimals(2);
    endExactTimeSpinBox->setValue(1000.0);
    inputLayout2->addWidget(endExactTimeLabel);
    inputLayout2->addWidget(endExactTimeSpinBox);

    groupBoxLayout2->addLayout(inputLayout2);
    groupBoxesLayout->addWidget(inputGroupBox2);

    mainLayout->addLayout(groupBoxesLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    buttonLayout->setAlignment(Qt::AlignLeft);

    QPushButton *createModelButton = new QPushButton("Создать", this);
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
        double endExactTime = endExactTimeSpinBox->value();
        double startExactTime = startExactTimeSpinBox->value();

        m_model= new StiffOde::StiffOdeModel(this);
        m_model->setInitialConditions({7, 13}, startTime);
        m_model->setParameters(stepSize, endTime, endExactTime, startExactTime);
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
