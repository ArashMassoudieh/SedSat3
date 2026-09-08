#include "plotwindow.h"
#include "ui_plotwindow.h"
#include "QComboBox"
#include "mainwindow.h"

PlotWindow::PlotWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlotWindow)
{
    ui->setupUi(this);

#ifdef USE_QCHARTS
    plot = new GeneralChartPlotter(this);
#else
	plot = new GeneralPlotter(this);    
#endif // USE_QCHARTS

    
    ui->horizontalLayout_2->addWidget(plot);

}

PlotWindow::~PlotWindow()
{
    delete ui;
}

SourceSinkData *PlotWindow::Data()
{
    return dynamic_cast<MainWindow*>(parent())->Data();
}


