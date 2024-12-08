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
    QtCharts::QLineSeries* getSeriesX() const;

private slots:
    void updateChart();

private:
    EulerModel *m_eulerModel;
    QtCharts::QChart *m_chartX;
    QtCharts::QChart *m_chartDxdt;
    QtCharts::QLineSeries *m_seriesEuler;
    QtCharts::QLineSeries *m_seriesHeun;
    QtCharts::QChartView *m_chartViewX;
    QtCharts::QChartView *m_chartViewDxdt;
    QDoubleSpinBox *m_aSpinBox;
    QSpinBox *m_timeSpinBox;
    QPushButton *m_updateButton;

    QtCharts::QValueAxis *m_axisX;
    QtCharts::QValueAxis *m_axisY;
    QtCharts::QValueAxis *m_axisX_dxdt;
    QtCharts::QValueAxis *m_axisY_dxdt;
};

}

