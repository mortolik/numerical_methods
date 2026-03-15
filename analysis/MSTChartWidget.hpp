#pragma once
#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <vector>

class MSTChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit MSTChartWidget(QWidget* parent = nullptr);
    void setData(const std::vector<double>& noise, const std::vector<double>& mst, bool withSignal);
private:
    QtCharts::QChartView* chartView_;
    QtCharts::QLineSeries* seriesWith_;
    QtCharts::QLineSeries* seriesWithout_;
    QtCharts::QChart* chart_;
};
