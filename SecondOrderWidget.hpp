#pragma once
#include <QWidget>
#include <QtCharts>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include "SecondOrderModel.hpp"

using namespace QtCharts;

class SecondOrderWidget : public QWidget {
    Q_OBJECT

public:
    SecondOrderWidget(SecondOrderModel *model, QWidget *parent = nullptr);

private slots:
    void runSimulation();
    void runMSTvsNoiseExperiment();


private:
    SecondOrderModel *m_model;
    QDoubleSpinBox *m_aSpinBox;
    QDoubleSpinBox *m_gammaSpinBox;
    QSpinBox *m_timeSpinBox;

    QtCharts::QChart *m_chart;
    QtCharts::QChartView *m_chartView;
    QtCharts::QLineSeries *m_series;
    QtCharts::QLineSeries *m_seriesClean;

    QPushButton *m_runButton;
    QPushButton *m_mstVsNoiseButton;
    QLabel *m_resultLabel;

    QCheckBox *m_useHeunCheckBox;

    // Для MST-эксперимента
    QDoubleSpinBox *m_dMinSpinBox;
    QDoubleSpinBox *m_dMaxSpinBox;
    QDoubleSpinBox *m_dStepSpinBox;
    QDoubleSpinBox *m_thresholdSpinBox;
    QSpinBox *m_trialsSpinBox;
    QCheckBox *m_switchingSignalCheckBox;
    QDoubleSpinBox *m_switchingAmplitudeSpinBox;
    QDoubleSpinBox *m_switchingFrequencySpinBox;

    // Для графика зависимости MST от шума
    QtCharts::QChart *m_mstChart;
    QtCharts::QChartView *m_mstChartView;
    QtCharts::QLineSeries *m_mstSeries;

};
