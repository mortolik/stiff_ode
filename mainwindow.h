#pragma once

#include "qspinbox.h"
#include <QMainWindow>

QT_FORWARD_DECLARE_CLASS(QHBoxLayout);

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
    QHBoxLayout* createGroupbox();

    StiffOde::StiffOdeModel* m_model = nullptr;
    StiffOde::StiffOdeWidget* m_widget = nullptr;

    QDoubleSpinBox* m_stepSizeSpinBox;
    QDoubleSpinBox* m_startTimeSpinBox;
    QDoubleSpinBox* m_endTimeSpinBox;
    QDoubleSpinBox* m_endExactTimeSpinBox;
    QDoubleSpinBox* m_startExactTimeSpinBox;
};
