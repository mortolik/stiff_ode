#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "StiffOdeModel.hpp"
#include "StiffOdeWidget.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    StiffOde::StiffOdeModel* model = new StiffOde::StiffOdeModel(this);
    model->setInitialConditions({7, 13}, 0.0);
    model->setParameters(0.005, 0.1);
    model->solve();

    StiffOde::StiffOdeWidget* stiffOdeWidget = new StiffOde::StiffOdeWidget(model, this);

    setCentralWidget(stiffOdeWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}
