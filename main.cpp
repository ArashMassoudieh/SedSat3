// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.


#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    //QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
    //    if (qgetenv("QT_FONT_DPI").isEmpty()) {
    //        qputenv("QT_FONT_DPI", "84");
    //    }
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
