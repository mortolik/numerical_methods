#pragma once
#include <QMainWindow>

namespace Euler{
class EulerModel;
class EulerWidget;
}
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QWidget* mainWidget;
    Euler::EulerModel* m_eulerModel {nullptr};
    Euler::EulerWidget* m_eulerWidget {nullptr};
};
