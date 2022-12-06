#ifndef GENERALCHART_H
#define GENERALCHART_H

#include <QDialog>
#include <qchartview.h>
#include <qchart.h>
#include "results.h"

#ifndef Qt6
using namespace QtCharts;
#endif

namespace Ui {
class GeneralChart;
}

class GeneralChart : public QDialog
{
    Q_OBJECT

public:
    explicit GeneralChart(QWidget *parent = nullptr);
    ~GeneralChart();
    bool Plot(result_item* res);
private:
    Ui::GeneralChart *ui;
    QChart* chart;
    QChartView *chartView;
};

double roundDown(double a);
#endif // GENERALCHART_H
