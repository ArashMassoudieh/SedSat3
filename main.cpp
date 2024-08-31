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
