#include "MainWindow.hpp"
#include "Euler/EulerModel.hpp"
#include "Euler/EulerWidget.hpp"
#include "Heun/HeunModel.hpp"
#include "Heun/HeunWidget.hpp"
#include <QHBoxLayout>

#include "MainWindow.hpp"
#include <QTabWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtCharts/QValueAxis>

using namespace QtCharts;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setObjectName("mainWindow");

    // Создаём виджет вкладок
    QTabWidget *tabWidget = new QTabWidget(this);

    // Настраиваем модель и виджет для Эйлера
    m_eulerModel = new Euler::EulerModel(0.5, 1000);
    m_eulerModel->setDt(0.01);
    m_eulerWidget = new Euler::EulerWidget(m_eulerModel, this);

    // Настраиваем модель и виджет для Хьюна
    m_heunModel = new Heun::HeunModel(0.5, 1000);
    m_heunModel->setDt(0.01);
    m_heunWidget = new Heun::HeunWidget(m_heunModel, this);

    // Добавляем вкладки
    tabWidget->addTab(m_eulerWidget, "Метод Эйлера");
    tabWidget->addTab(m_heunWidget, "Метод Хьюна");

    // Создаём серию и график для отображения разницы
    m_diffSeriesX = new QLineSeries();
    m_diffChart = new QChart();
    m_diffChart->addSeries(m_diffSeriesX);
    m_diffChart->setTitle("Разница x(t) между методами");

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("t");
    m_diffChart->addAxis(axisX, Qt::AlignBottom);
    m_diffSeriesX->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Разница");
    m_diffChart->addAxis(axisY, Qt::AlignLeft);
    m_diffSeriesX->attachAxis(axisY);

    m_diffChartView = new QChartView(m_diffChart);
    m_diffChartView->setRenderHint(QPainter::Antialiasing);

    // Кнопка для обновления разницы
    QPushButton *calculateDiffButton = new QPushButton("Показать разницу");
    connect(calculateDiffButton, &QPushButton::clicked, this, [this]() {
        calculateDifferences(m_eulerWidget->getSeriesX(), m_heunWidget->getSeriesX(), m_diffSeriesX);
    });

    // Основной макет
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tabWidget);
    layout->addWidget(m_diffChartView);
    layout->addWidget(calculateDiffButton);

    QWidget *mainWidget = new QWidget(this);
    mainWidget->setLayout(layout);
    setCentralWidget(mainWidget);

    setMinimumSize(800, 600);
}

void MainWindow::calculateDifferences(QLineSeries *eulerSeries, QLineSeries *heunSeries, QLineSeries *diffSeries) {
    diffSeries->clear();

    const auto eulerPoints = eulerSeries->pointsVector();
    const auto heunPoints = heunSeries->pointsVector();

    int pointsCount = std::min(eulerPoints.size(), heunPoints.size());
    for (int i = 0; i < pointsCount; ++i) {
        double t = eulerPoints[i].x();
        double diff = std::abs(eulerPoints[i].y() - heunPoints[i].y());
        diffSeries->append(t, diff); // Разница между методами
    }
}


MainWindow::~MainWindow() {
    delete m_eulerModel;
    delete m_eulerWidget;
}
