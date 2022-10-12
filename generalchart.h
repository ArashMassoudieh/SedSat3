#ifndef GENERALCHART_H
#define GENERALCHART_H

#include <QDialog>
#include <qchartview.h>
#include <qchart.h>


namespace Ui {
class GeneralChart;
}

class GeneralChart : public QDialog
{
    Q_OBJECT

public:
    explicit GeneralChart(QWidget *parent = nullptr);
    ~GeneralChart();

private:
    Ui::GeneralChart *ui;
    QtCharts::QChart* chart;
    QtCharts::QChartView *chartView;
};

#endif // GENERALCHART_H
