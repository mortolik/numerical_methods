#pragma once

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

namespace QtCharts {
class QChartView;
}

class QDoubleSpinBox;
class QSpinBox;
class QPushButton;

namespace Euler {

class EulerModel;

class EulerWidget : public QWidget {
    Q_OBJECT

public:
    explicit EulerWidget(EulerModel *model, QWidget *parent = nullptr);

private slots:
    void updateChart();

    void computeSwitchDelay();
private:
    EulerModel *m_eulerModel;
    QtCharts::QChart *m_chartX;
    QtCharts::QChart *m_chartDxdt;
    QtCharts::QLineSeries *m_seriesX;
    QtCharts::QLineSeries *m_seriesDxdt;
    QtCharts::QChartView *m_chartViewX;
    QtCharts::QChartView *m_chartViewDxdt;
    QDoubleSpinBox *m_aSpinBox;
    QSpinBox *m_timeSpinBox;
    QPushButton *m_updateButton;
    QPushButton *m_delayButton;

    QtCharts::QValueAxis *m_axisX_x;
    QtCharts::QValueAxis *m_axisY_x;
    QtCharts::QValueAxis *m_axisX_dxdt;
    QtCharts::QValueAxis *m_axisY_dxdt;
};

}

