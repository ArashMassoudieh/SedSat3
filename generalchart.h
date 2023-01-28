#ifndef GENERALCHART_H
#define GENERALCHART_H

#include <QDialog>
#include <qchartview.h>
#include <qchart.h>
#include "results.h"
#include "cmbvector.h"
#include <contribution.h>
#include <elemental_profile_set.h>
#include <multiplelinearregressionset.h>

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
    bool Plot(ResultItem* res);
    bool PlotVector(CMBVector *profile, const QString &title);
    bool PlotProfileSet(Elemental_Profile_Set *elementalprofileset, const QString &title);
    bool PlotContribution(Contribution* contributions, const QString &title);
    bool PlotPredictedConcentration(Elemental_Profile* elemental_profile, const QString &title);
    bool PlotRegressionSet(MultipleLinearRegressionSet *regressionset, const QString &title);
private:
    Ui::GeneralChart *ui;
    QChart* chart;
    QChartView *chartView;
};

double roundDown(double a, double rounding_number=1.0);
#endif // GENERALCHART_H
