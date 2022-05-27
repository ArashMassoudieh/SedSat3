#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void SetSinkSheet(int i) {Sink_Sheet = i;}
    void WriteMessageOnScreen(const QString &text, QColor color=Qt::black);
private:
    Ui::MainWindow *ui;
    int Sink_Sheet=-1;
private slots:
    void on_import_excel();
    bool ReadExcel(const QString &filename);

};
#endif // MAINWINDOW_H
