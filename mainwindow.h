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
    Ui::MainWindow *ui;
    StiffOde::StiffOdeModel* m_model {nullptr};
    StiffOde::StiffOdeWidget* m_widget {nullptr};

    QDoubleSpinBox * m_stepSizeSpinBox {nullptr};
    QDoubleSpinBox * m_startTimeSpinBox {nullptr};
    QDoubleSpinBox * m_endTimeSpinBox {nullptr};
    QDoubleSpinBox * m_endExactTimeSpinBox {nullptr};
    QDoubleSpinBox * m_startExactTimeSpinBox {nullptr};

    QHBoxLayout* createGroupbox();
};
