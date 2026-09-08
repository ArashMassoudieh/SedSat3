#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H
#define USE_QCHARTS

#include <QDialog>
#include "sourcesinkdata.h"
#include "QComboBox"
#ifdef USE_QCHARTS
#include "GeneralChartPlotter.h"
#else
#include "generalplotter.h"
#endif // USE_QCHARTS

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
#ifdef USE_QCHARTS
    GeneralChartPlotter *Plotter() {return plot;}
#else
	GeneralPlotter* Plotter() { return chartplotter; }
#endif
private:
    Ui::PlotWindow *ui;
#ifdef USE_QCHARTS
	GeneralChartPlotter* plot = nullptr;
#else
    GeneralPlotter *plot = nullptr;
#endif


public slots:

};

#endif // PLOTWINDOW_H
