#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QDialog>
#include "sourcesinkdata.h"
#include "QComboBox"
#include "generalplotter.h"

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
    GeneralPlotter *Plotter() {return plot;}
private:
    Ui::PlotWindow *ui;
    GeneralPlotter *plot = nullptr;

public slots:

};

#endif // PLOTWINDOW_H
