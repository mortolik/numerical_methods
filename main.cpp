#include "MainWindow.hpp"
#include "qapplication.h"
#include <QDir>
#include <QFileInfo>
#include <QTimer>
#include <QDebug>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("numerical_methods");
    QCoreApplication::setApplicationName("numerical_method");


    MainWindow w;

    QStringList args = QCoreApplication::arguments();
    if (args.size() == 4 && args[1] == "--export-preset") {
        bool ok = false;
        int presetIndex = args[2].toInt(&ok);
        QString outputPath = QFileInfo(args[3]).absoluteFilePath();

        if (!ok || presetIndex <= 0) {
            qCritical() << "Invalid preset index:" << args[2];
            return 2;
        }

        QDir().mkpath(QFileInfo(outputPath).absolutePath());
        w.show();
        QTimer::singleShot(0, &w, [&a, &w, presetIndex, outputPath]() {
            bool exported = w.exportSecondOrderPreset(presetIndex, outputPath);
            if (!exported) {
                qCritical() << "Failed to export preset" << presetIndex << "to" << outputPath;
            }
            a.exit(exported ? 0 : 1);
        });
        return a.exec();
    }

    if (args.size() == 7 && args[1] == "--export-preset-range") {
        bool okPreset = false;
        bool okMin = false;
        bool okMax = false;
        bool okPoints = false;
        int presetIndex = args[2].toInt(&okPreset);
        QString outputPath = QFileInfo(args[3]).absoluteFilePath();
        double dMin = args[4].toDouble(&okMin);
        double dMax = args[5].toDouble(&okMax);
        int points = args[6].toInt(&okPoints);

        if (!okPreset || !okMin || !okMax || !okPoints || presetIndex <= 0 || dMin <= 0.0 || dMax <= dMin || points < 5) {
            qCritical() << "Invalid args for --export-preset-range";
            return 2;
        }

        QDir().mkpath(QFileInfo(outputPath).absolutePath());
        w.show();
        QTimer::singleShot(0, &w, [&a, &w, presetIndex, outputPath, dMin, dMax, points]() {
            bool exported = w.exportSecondOrderPreset(presetIndex, outputPath, dMin, dMax, points);
            if (!exported) {
                qCritical() << "Failed to export preset" << presetIndex << "to" << outputPath
                            << "range:" << dMin << dMax << "points:" << points;
            }
            a.exit(exported ? 0 : 1);
        });
        return a.exec();
    }

    w.show();

    return a.exec();
}
