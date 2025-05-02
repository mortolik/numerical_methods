#include "SecondOrderWidget.hpp"
#include <QVBoxLayout>
#include <QMessageBox>

SecondOrderWidget::SecondOrderWidget(SecondOrderModel *model, QWidget *parent)
    : QWidget{parent}, m_model(model)
{
    m_aSpinBox = new QDoubleSpinBox();
    m_aSpinBox->setRange(-10.0, 10.0);
    m_aSpinBox->setSingleStep(0.1);
    m_aSpinBox->setPrefix("a = ");
    m_aSpinBox->setValue(0.5);

    m_gammaSpinBox = new QDoubleSpinBox();
    m_gammaSpinBox->setRange(0.01, 10.0);
    m_gammaSpinBox->setSingleStep(0.1);
    m_gammaSpinBox->setPrefix("γ = ");
    m_gammaSpinBox->setValue(0.5);

    m_timeSpinBox = new QSpinBox();
    m_timeSpinBox->setRange(1, 1000);
    m_timeSpinBox->setPrefix("Время = ");
    m_timeSpinBox->setValue(1000);

    m_computeButton = new QPushButton("Посчитать задержку включения");

    connect(m_computeButton, &QPushButton::clicked, this, &SecondOrderWidget::computeSwitchDelay);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_aSpinBox);
    layout->addWidget(m_gammaSpinBox);
    layout->addWidget(m_timeSpinBox);
    layout->addWidget(m_computeButton);
    setLayout(layout);
}

void SecondOrderWidget::computeSwitchDelay() {
    double a = m_aSpinBox->value();
    double gamma = m_gammaSpinBox->value();
    int maxTime = m_timeSpinBox->value();

    m_model->setA(a);
    m_model->setGamma(gamma);
    double dt = static_cast<double>(maxTime) / 1000;
    m_model->setDt(dt);

    double threshold = M_PI;
    int trials = 100;

    double delay = m_model->computeSwitchDelay(threshold, trials);

    QMessageBox::information(this, "Задержка включения (2 порядок)",
                             QString("Средняя задержка по %1 траекториям: %2").arg(trials).arg(delay, 0, 'f', 4));
}
