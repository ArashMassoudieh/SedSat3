#include "generalchart.h"
#include "ui_generalchart.h"
#include <QtCharts>

GeneralChart::GeneralChart(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GeneralChart)
{
    ui->setupUi(this);
    chart = new QtCharts::QChart();
    chartView = new QtCharts::QChartView(chart);

}

GeneralChart::~GeneralChart()
{
    delete ui;
}
