#include "MSTChartWidget.hpp"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QLogValueAxis>
#include <QVBoxLayout>

MSTChartWidget::MSTChartWidget(QWidget* parent)
    : QWidget(parent),
      chartView_(new QtCharts::QChartView(this)),
      seriesWith_(new QtCharts::QLineSeries()),
      seriesWithout_(new QtCharts::QLineSeries()),
      chart_(new QtCharts::QChart())
{
    chart_->addSeries(seriesWith_);
    chart_->addSeries(seriesWithout_);
    chart_->setTitle("Среднее время переключения vs интенсивность шума");
    
    auto* axisX = new QtCharts::QValueAxis();
    axisX->setTitleText("Интенсивность шума D");
    chart_->addAxis(axisX, Qt::AlignBottom);
    seriesWith_->attachAxis(axisX);
    seriesWithout_->attachAxis(axisX);

    auto* axisY = new QtCharts::QLogValueAxis();
    axisY->setTitleText("Среднее время переключения (log)");
    axisY->setBase(10.0);
    chart_->addAxis(axisY, Qt::AlignLeft);
    seriesWith_->attachAxis(axisY);
    seriesWithout_->attachAxis(axisY);

    chartView_->setChart(chart_);
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(chartView_);
    setLayout(layout);
    seriesWith_->setName("С сигналом");
    seriesWithout_->setName("Без сигнала");
}

void MSTChartWidget::setData(const std::vector<double>& noise, const std::vector<double>& mst, bool withSignal) {
    auto* series = withSignal ? seriesWith_ : seriesWithout_;
    series->clear();
    double minX = 1e9, maxX = -1e9;
    double minY = 1e9, maxY = -1e9;
    
    for (size_t i = 0; i < noise.size() && i < mst.size(); ++i) {
        if (mst[i] > 0) {
            series->append(noise[i], mst[i]);
        }
    }
    
    // Automatically adjust axes
    for (const auto* s : {seriesWith_, seriesWithout_}) {
        for (const QPointF& p : s->points()) {
            minX = qMin(minX, p.x());
            maxX = qMax(maxX, p.x());
            minY = qMin(minY, p.y());
            maxY = qMax(maxY, p.y());
        }
    }
    
    if (minX <= maxX && minY <= maxY && minY > 0) {
        auto* axisX = qobject_cast<QtCharts::QValueAxis*>(chart_->axes(Qt::Horizontal).first());
        if (axisX) {
            axisX->setRange(minX, maxX);
        }
        auto* axisY = qobject_cast<QtCharts::QLogValueAxis*>(chart_->axes(Qt::Vertical).first());
        if (axisY) {
            axisY->setRange(minY * 0.9, maxY * 1.1);
        }
    }
}
