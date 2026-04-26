#pragma once
#include "SecondOrderModel.hpp"
#include "SecondOrderWidget.hpp"
#include <QMainWindow>

namespace Euler
{
class EulerModel;
class EulerWidget;
}
namespace Heun
{
class HeunModel;
class HeunWidget;
}
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MSTChartWidget;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool exportSecondOrderPreset(
        int presetIndex,
        const QString &filePath,
        double dMinOverride = -1.0,
        double dMaxOverride = -1.0,
        int pointsOverride = -1);

private slots:
    void onRunMSTAnalysis();

private:
    QWidget* mainWidget;
    Euler::EulerModel* m_eulerModel {nullptr};
    Euler::EulerWidget* m_eulerWidget {nullptr};

    Heun::HeunModel* m_heunModel {nullptr};
    Heun::HeunWidget* m_heunWidget {nullptr};

    SecondOrderModel *m_secondOrderModel {nullptr};
    SecondOrderWidget *m_secondOrderWidget {nullptr};

    MSTChartWidget* m_mstChartWidget {nullptr};
};
