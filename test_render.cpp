#include <QApplication>
#include <QChartView>
#include <QLineSeries>
#include <QPixmap>
#include <QPainter>

using namespace QtCharts;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QChart *chart = new QChart();
    QLineSeries *series = new QLineSeries();
    series->append(0, 0); series->append(1, 1);
    chart->addSeries(series);
    chart->createDefaultAxes();
    QChartView *view = new QChartView(chart);
    
    QPixmap pixmap(1600, 1200);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    view->resize(1600, 1200);
    view->scene()->render(&painter);
    
    pixmap.save("test_render.png");
    return 0;
}
