#include "generalchart.h"
#include "ui_generalchart.h"
#include <QtCharts>

GeneralChart::GeneralChart(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GeneralChart)
{
    ui->setupUi(this);
    chartView = new QtCharts::QChartView(chart);
    chart = new QtCharts::QChart();
}

GeneralChart::~GeneralChart()
{
    delete ui;
}
