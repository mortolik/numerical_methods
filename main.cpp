#include <QApplication>
#include <QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <cmath>

using namespace QtCharts;

double dxdt(double x, double a) {
    return a - sin(x);
}

void eulerMethod(double a, double x0, double t0, double dt, int steps, QLineSeries *series_x, QLineSeries *series_dxdt) {
    double x = x0;
    double t = t0;
    series_x->clear();
    series_dxdt->clear();

    for (int i = 0; i < steps; ++i) {
        double dx_dt = dxdt(x, a);

        series_x->append(t, x);
        series_dxdt->append(t, dx_dt);

        x += dt * dx_dt;
        t += dt;
    }
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QMainWindow window;
    QWidget *centralWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    double x0 = 0;
    double t0 = 0;
    double dt = 0.01;
    int steps = 1000;
    double a = 0.5;

    QChart *chart_x = new QChart();
    chart_x->setTitle("График x(t)");

    QChart *chart_dxdt = new QChart();
    chart_dxdt->setTitle("График dx/dt");

    QLineSeries *series_x = new QLineSeries();
    series_x->setName("x(t)");

    QLineSeries *series_dxdt = new QLineSeries();
    series_dxdt->setName("dx/dt");

    eulerMethod(a, x0, t0, dt, steps, series_x, series_dxdt);

    chart_x->addSeries(series_x);
    chart_dxdt->addSeries(series_dxdt);

    QValueAxis *axisX_x = new QValueAxis();
    axisX_x->setTitleText("t");
    axisX_x->setLabelFormat("%.2f");
    chart_x->addAxis(axisX_x, Qt::AlignBottom);

    QValueAxis *axisY_x = new QValueAxis();
    axisY_x->setTitleText("x(t)");
    axisY_x->setLabelFormat("%.2f");
    chart_x->addAxis(axisY_x, Qt::AlignLeft);

    series_x->attachAxis(axisX_x);
    series_x->attachAxis(axisY_x);

    QValueAxis *axisX_dxdt = new QValueAxis();
    axisX_dxdt->setTitleText("t");
    axisX_dxdt->setLabelFormat("%.2f");
    chart_dxdt->addAxis(axisX_dxdt, Qt::AlignBottom);

    QValueAxis *axisY_dxdt = new QValueAxis();
    axisY_dxdt->setTitleText("dx/dt");
    axisY_dxdt->setLabelFormat("%.2f");
    chart_dxdt->addAxis(axisY_dxdt, Qt::AlignLeft);

    series_dxdt->attachAxis(axisX_dxdt);
    series_dxdt->attachAxis(axisY_dxdt);

    QChartView *chartView_x = new QChartView(chart_x);
    chartView_x->setRenderHint(QPainter::Antialiasing);

    QChartView *chartView_dxdt = new QChartView(chart_dxdt);
    chartView_dxdt->setRenderHint(QPainter::Antialiasing);

    QDoubleSpinBox *aSpinBox = new QDoubleSpinBox();
    aSpinBox->setRange(-10.0, 10.0);
    aSpinBox->setValue(a);
    aSpinBox->setSingleStep(0.1);
    aSpinBox->setPrefix("a = ");

    QPushButton *updateButton = new QPushButton("Обновить графики");

    QObject::connect(updateButton, &QPushButton::clicked, [&]() {
        double newA = aSpinBox->value();
        eulerMethod(newA, x0, t0, dt, steps, series_x, series_dxdt);
    });

    layout->addWidget(chartView_x);
    layout->addWidget(chartView_dxdt);
    layout->addWidget(aSpinBox);
    layout->addWidget(updateButton);

    window.setCentralWidget(centralWidget);

    window.resize(800, 800);
    window.show();

    return app.exec();
}
