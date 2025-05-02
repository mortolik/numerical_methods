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
    void computeSwitchDelay();

private:
    SecondOrderModel *m_model;
    QDoubleSpinBox *m_aSpinBox;
    QDoubleSpinBox *m_gammaSpinBox;
    QSpinBox *m_timeSpinBox;
    QPushButton *m_computeButton;
};
