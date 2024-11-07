#include "EulerWidget.hpp"
#include "Euler/EulerModel.hpp"
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QVBoxLayout>

using namespace QtCharts;

namespace Euler
{

EulerWidget::EulerWidget(EulerModel *model, QWidget *parent)
    : QWidget{parent}
    , m_eulerModel(model)
{
    QChart *chart_x = new QChart();
    chart_x->setTitle("График x(t)");

    QChart *chart_dxdt = new QChart();
    chart_dxdt->setTitle("График dx/dt");

    QLineSeries *series_x = new QLineSeries();
    series_x->setName("x(t)");

    QLineSeries *series_dxdt = new QLineSeries();
    series_dxdt->setName("dx/dt");

    m_eulerModel->method(series_x, series_dxdt);

    chart_x->addSeries(series_x);
    chart_dxdt->addSeries(series_dxdt);

    QValueAxis *axisX_x = new QValueAxis();
    axisX_x->setTitleText("t");
    axisX_x->setLabelFormat("%.2f");
    axisX_x->setRange(0, 10);

    QValueAxis *axisY_x = new QValueAxis();
    axisY_x->setTitleText("x(t)");
    axisY_x->setLabelFormat("%.2f");

    chart_x->addAxis(axisX_x, Qt::AlignBottom);
    chart_x->addAxis(axisY_x, Qt::AlignLeft);

    series_x->attachAxis(axisX_x);
    series_x->attachAxis(axisY_x);

    QValueAxis *axisX_dxdt = new QValueAxis();
    axisX_dxdt->setTitleText("t");
    axisX_dxdt->setLabelFormat("%.2f");
    axisX_dxdt->setRange(0, 10);

    QValueAxis *axisY_dxdt = new QValueAxis();
    axisY_dxdt->setTitleText("dx/dt");
    axisY_dxdt->setLabelFormat("%.2f");

    chart_dxdt->addAxis(axisX_dxdt, Qt::AlignBottom);
    chart_dxdt->addAxis(axisY_dxdt, Qt::AlignLeft);

    series_dxdt->attachAxis(axisX_dxdt);
    series_dxdt->attachAxis(axisY_dxdt);

    QChartView *chartView_x = new QChartView(chart_x);
    chartView_x->setRenderHint(QPainter::Antialiasing);

    QChartView *chartView_dxdt = new QChartView(chart_dxdt);
    chartView_dxdt->setRenderHint(QPainter::Antialiasing);

    QDoubleSpinBox *aSpinBox = new QDoubleSpinBox();
    aSpinBox->setRange(-10.0, 10.0);
    aSpinBox->setValue(m_eulerModel->getA());
    aSpinBox->setSingleStep(0.1);
    aSpinBox->setPrefix("a = ");

    QPushButton *updateButton = new QPushButton("Обновить графики");

    QObject::connect(updateButton, &QPushButton::clicked, [&]() {
        double newA = aSpinBox->value();
        m_eulerModel->setA(newA);
        m_eulerModel->method(series_x, series_dxdt);
    });

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(chartView_x);
    mainLayout->addWidget(chartView_dxdt);
    mainLayout->addWidget(aSpinBox);
    mainLayout->addWidget(updateButton);

    setLayout(mainLayout);
}

}
