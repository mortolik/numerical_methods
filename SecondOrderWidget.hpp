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


private:
    SecondOrderModel *m_model;
    QDoubleSpinBox *m_aSpinBox;
    QDoubleSpinBox *m_gammaSpinBox;
    QSpinBox *m_timeSpinBox;

    QtCharts::QChart *m_chart;
    QtCharts::QChartView *m_chartView;
    QtCharts::QLineSeries *m_series;

    QPushButton *m_runButton;
    QLabel *m_resultLabel;

};
