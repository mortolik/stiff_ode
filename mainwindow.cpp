#include "mainwindow.h"
#include "StiffOdeModel.hpp"
#include "StiffOdeWidget.hpp"
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineSeries>
#include <QDoubleSpinBox>
#include <QtCharts/QChartView>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setObjectName("mainWindow");
    setMinimumSize(1200, 800); // Увеличен размер для удобства

    QWidget *centralWidget = new QWidget(this);
    QPalette pal = centralWidget->palette();
    pal.setColor(QPalette::Window, Qt::white);
    centralWidget->setAutoFillBackground(true);
    centralWidget->setPalette(pal);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setAlignment(Qt::AlignTop);
    mainLayout->setSpacing(10);

    mainLayout->addLayout(createGroupbox());

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
            m_widget = nullptr;
        }

        double stepSize = m_stepSizeSpinBox->value();
        double startTime = m_startTimeSpinBox->value();
        double endTime = m_endTimeSpinBox->value();
        double endExactTime = endTime; // Устанавливаем endExactTime равным endTime
        double startExactTime = m_startExactTimeSpinBox->value();

        if (startTime >= endTime) {
            qDebug() << "Начальное время должно быть меньше конечного.";
            return;
        }

        if (startExactTime > endExactTime) {
            qDebug() << "Начальное время точного решения должно быть меньше или равно конечному.";
            return;
        }

        m_model = new StiffOde::StiffOdeModel(this);
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

QHBoxLayout* MainWindow::createGroupbox()
{
    QHBoxLayout *groupBoxesLayout = new QHBoxLayout();
    groupBoxesLayout->setSpacing(10);
    groupBoxesLayout->setAlignment(Qt::AlignLeft);

    // Основные параметры
    QGroupBox *inputGroupBox1 = new QGroupBox("Основные параметры", this);
    QVBoxLayout *groupBoxLayout1 = new QVBoxLayout(inputGroupBox1);
    groupBoxLayout1->setSpacing(10);

    QHBoxLayout *inputLayout1 = new QHBoxLayout();
    inputLayout1->setSpacing(10);

    QLabel *stepSizeLabel = new QLabel("Размер шага:", this);
    m_stepSizeSpinBox = new QDoubleSpinBox(this);
    m_stepSizeSpinBox->setRange(0.001, 1.0);
    m_stepSizeSpinBox->setDecimals(3);
    m_stepSizeSpinBox->setValue(0.010);
    m_stepSizeSpinBox->setSingleStep(0.001);

    inputLayout1->addWidget(stepSizeLabel);
    inputLayout1->addWidget(m_stepSizeSpinBox);

    QLabel *startTimeLabel = new QLabel("Начальное x_n:", this);
    m_startTimeSpinBox = new QDoubleSpinBox(this);
    m_startTimeSpinBox->setRange(0, 100000);
    m_startTimeSpinBox->setDecimals(3);
    m_startTimeSpinBox->setValue(0.0);
    m_startTimeSpinBox->setSingleStep(0.1);
    inputLayout1->addWidget(startTimeLabel);
    inputLayout1->addWidget(m_startTimeSpinBox);

    QLabel *endTimeLabel = new QLabel("Конечное x_n:", this);
    m_endTimeSpinBox = new QDoubleSpinBox(this);
    m_endTimeSpinBox->setRange(0, 100000);
    m_endTimeSpinBox->setDecimals(3);
    m_endTimeSpinBox->setValue(10000.0);
    m_endTimeSpinBox->setSingleStep(10.0);
    inputLayout1->addWidget(endTimeLabel);
    inputLayout1->addWidget(m_endTimeSpinBox);

    groupBoxLayout1->addLayout(inputLayout1);
    groupBoxesLayout->addWidget(inputGroupBox1);

    // Параметры точного решения
    QGroupBox *inputGroupBox2 = new QGroupBox("Параметры точного решения", this);
    QVBoxLayout *groupBoxLayout2 = new QVBoxLayout(inputGroupBox2);
    groupBoxLayout2->setSpacing(10);

    QHBoxLayout *inputLayout2 = new QHBoxLayout();
    inputLayout2->setSpacing(10);

    QLabel *startExactTimeLabel = new QLabel("Начальное x_n:", this);
    m_startExactTimeSpinBox = new QDoubleSpinBox(this);
    m_startExactTimeSpinBox->setRange(0, 100000);
    m_startExactTimeSpinBox->setDecimals(3);
    m_startExactTimeSpinBox->setValue(0.0);
    m_startExactTimeSpinBox->setSingleStep(0.1);
    inputLayout2->addWidget(startExactTimeLabel);
    inputLayout2->addWidget(m_startExactTimeSpinBox);

    QLabel *endExactTimeLabel = new QLabel("Конечное x_n:", this);
    m_endExactTimeSpinBox = new QDoubleSpinBox(this);
    m_endExactTimeSpinBox->setRange(0, 100000);
    m_endExactTimeSpinBox->setDecimals(3);
    m_endExactTimeSpinBox->setValue(10000.0); // Теперь соответствует endTime
    m_endExactTimeSpinBox->setSingleStep(10.0);
    inputLayout2->addWidget(endExactTimeLabel);
    inputLayout2->addWidget(m_endExactTimeSpinBox);

    groupBoxLayout2->addLayout(inputLayout2);
    groupBoxesLayout->addWidget(inputGroupBox2);

    return groupBoxesLayout;
}
