#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QDialog>
#include "sourcesinkdata.h"

namespace Ui {
class PlotWindow;
}

class PlotWindow : public QDialog
{
    Q_OBJECT

public:
    explicit PlotWindow(QWidget *parent = nullptr);
    ~PlotWindow();
    SourceSinkData *Data();

private:
    Ui::PlotWindow *ui;
};

#endif // PLOTWINDOW_H
