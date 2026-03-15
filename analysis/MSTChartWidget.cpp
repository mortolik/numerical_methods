#include "MSTChartWidget.hpp"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
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
    chart_->createDefaultAxes();
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
    for (size_t i = 0; i < noise.size() && i < mst.size(); ++i) {
        series->append(noise[i], mst[i]);
    }
    chart_->createDefaultAxes();
}
