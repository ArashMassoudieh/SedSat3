#include "plotwindow.h"
#include "ui_plotwindow.h"
#include "qcustomplot.h"
#include "QComboBox"
#include "mainwindow.h"

PlotWindow::PlotWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlotWindow)
{
    ui->setupUi(this);
    QComboBox* combo_groups = new QComboBox(this);
    QComboBox* combo_sample = new QComboBox(this);
    for (int i=0; i<Data()->GroupNames().size(); i++)
    {
        combo_groups->addItem(QString::fromStdString(Data()->GroupNames()[i]));
    }

    ui->horizontalLayout->addWidget(combo_groups);
    ui->horizontalLayout->addWidget(combo_sample);


}

PlotWindow::~PlotWindow()
{
    delete ui;
}

SourceSinkData *PlotWindow::Data()
{
    return dynamic_cast<MainWindow*>(parent())->Data();
}
