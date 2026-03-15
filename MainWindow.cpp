#include "MainWindow.hpp"
#include "Euler/EulerModel.hpp"
#include "Euler/EulerWidget.hpp"
#include "Heun/HeunModel.hpp"
#include "Heun/HeunWidget.hpp"
#include "SecondOrderModel.hpp"
#include "SecondOrderWidget.hpp"
#include "analysis/SwitchingAnalysis.hpp"
#include "analysis/MSTChartWidget.hpp"
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setObjectName("mainWindow");

    QTabWidget *tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);

    m_eulerModel = new Euler::EulerModel(0.5, 1000);
    m_eulerModel->setDt(0.01);
    m_eulerWidget = new Euler::EulerWidget(m_eulerModel, this);

    m_heunModel = new Heun::HeunModel(0.5, 1000);
    m_heunModel->setDt(0.01);
    m_heunWidget = new Heun::HeunWidget(m_heunModel, this);

    tabWidget->addTab(m_eulerWidget, "Метод Эйлера");
    tabWidget->addTab(m_heunWidget, "Метод Хьюна");

    m_secondOrderModel = new SecondOrderModel(0.5, 0.5, 1000);
    m_secondOrderModel->setDt(0.01);
    m_secondOrderWidget = new SecondOrderWidget(m_secondOrderModel, this);
    tabWidget->addTab(m_secondOrderWidget, "2-й порядок");

    // --- MST Analysis Tab ---
    QWidget* mstTab = new QWidget(this);
    QVBoxLayout* mstLayout = new QVBoxLayout(mstTab);
    QPushButton* runBtn = new QPushButton("Построить MST-график", mstTab);
    m_mstChartWidget = new MSTChartWidget(mstTab);
    mstLayout->addWidget(runBtn);
    mstLayout->addWidget(m_mstChartWidget);
    mstTab->setLayout(mstLayout);
    tabWidget->addTab(mstTab, "MST-анализ");
    connect(runBtn, &QPushButton::clicked, this, &MainWindow::onRunMSTAnalysis);

    setMinimumSize(800, 600);
}

MainWindow::~MainWindow() {
    delete m_eulerModel;
    delete m_eulerWidget;
    delete m_heunModel;
    delete m_heunWidget;
    delete m_secondOrderModel;
    delete m_secondOrderWidget;
    delete m_mstChartWidget;
}

void MainWindow::onRunMSTAnalysis() {
    // Параметры моделирования
    double dt = 0.01;
    int maxSteps = 10000;
    int nRuns = 300;
    std::vector<double> noiseList;
    for (double D = 0.01; D <= 0.2; D += 0.01) noiseList.push_back(D);
    SwitchingAnalysis analysis(dt, maxSteps);
    // Без сигнала
    auto mstNoSignal = analysis.mstVsNoise(noiseList, false, nRuns);
    m_mstChartWidget->setData(noiseList, mstNoSignal, false);
    // С сигналом
    auto mstWithSignal = analysis.mstVsNoise(noiseList, true, nRuns);
    m_mstChartWidget->setData(noiseList, mstWithSignal, true);
}
