#pragma once
#include <QMainWindow>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

using namespace QtCharts;

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

    Heun::HeunModel* m_heunModel {nullptr};
    Heun::HeunWidget* m_heunWidget {nullptr};
    void calculateDifferences(QLineSeries *eulerSeries, QLineSeries *heunSeries, QLineSeries *diffSeries);

    // График разницы
    QLineSeries *m_diffSeriesX;
    QChart *m_diffChart;
    QChartView *m_diffChartView;
};
