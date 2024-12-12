#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE
namespace StiffOde
{
class StiffOdeModel;
class StiffOdeWidget;
}
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    StiffOde::StiffOdeModel* m_model {nullptr};
    StiffOde::StiffOdeWidget* m_widget {nullptr};
};
