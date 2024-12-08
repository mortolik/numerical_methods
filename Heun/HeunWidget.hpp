#pragma once

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

namespace QtCharts
{
class QChartView;
}

class QDoubleSpinBox;
class QSpinBox;
class QPushButton;

namespace Heun
{
class HeunModel;
class HeunWidget : public QWidget {
    Q_OBJECT
public:
    explicit HeunWidget(HeunModel *model, QWidget *parent);

private slots:
    void updateChart();

private:
    HeunModel *m_heunModel;
    QtCharts::QChart *m_chartX;
    QtCharts::QChart *m_chartDxdt;
    QtCharts::QLineSeries *m_seriesX;
    QtCharts::QLineSeries *m_seriesDxdt;
    QtCharts::QChartView *m_chartViewX;
    QtCharts::QChartView *m_chartViewDxdt;
    QDoubleSpinBox *m_aSpinBox;
    QSpinBox *m_timeSpinBox;
    QPushButton *m_updateButton;

    QtCharts::QValueAxis *m_axisX_x;
    QtCharts::QValueAxis *m_axisY_x;
    QtCharts::QValueAxis *m_axisX_dxdt;
    QtCharts::QValueAxis *m_axisY_dxdt;
};

}