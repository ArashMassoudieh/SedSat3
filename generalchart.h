#ifndef GENERALCHART_H
#define GENERALCHART_H

#include <QDialog>
#include <qchartview.h>
#include <qchart.h>
#include "results.h"
#include "cmbvector.h"
#include "cmbvectorset.h"
#include <contribution.h>
#include <elemental_profile_set.h>
#include <multiplelinearregressionset.h>
#include <QComboBox>
#include "rangeset.h"
#include <chart.h>
#include <chartview.h>

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
    bool PlotVectorSet(CMBVectorSet *profile, const QString &title);
    bool PlotMatrix(CMBMatrix *matrix, const QString &title);
    bool PlotProfileSet(Elemental_Profile_Set *elementalprofileset, const QString &title);
    bool PlotContribution(Contribution* contributions, const QString &title);
    bool PlotPredictedConcentration(Elemental_Profile* elemental_profile, const QString &title);
    bool PlotRegressionSet(MultipleLinearRegressionSet *regressionset, const QString &title);
    bool PlotTimeSeriesSet(CMBTimeSeriesSet *regressionset, const QString &title, const QString &x_axis_title="", const QString &y_axis_title="");
    bool PlotTimeSeriesSet_M(CMBTimeSeriesSet *timeseriesset, const QString &title, const QString &x_axis_title="", const QString &y_axis_title="");
    bool PlotTimeSeriesSet_A(CMBTimeSeriesSet *timeseriesset, const QString &title, const QString &x_axis_title="", const QString &y_axis_title="");
    bool PlotTimeSeriesSet_Stacked(CMBTimeSeriesSet *timeseriesset, const QString &title, const QString &x_axis_title, const QString &y_axis_title="");
    bool PlotRangeSet(RangeSet *rangeset, const QString &title, const QString &x_axis_title, const QString &y_axis_title);
    bool InitializeMCMCSamples(CMBTimeSeriesSet *mcmcsamples, const QString &title);
    bool InitializeDistributions(CMBTimeSeriesSet *distributions, const QString &title);
    bool PlotMCMCSamples(CTimeSeries<double> *mlr,const QString& variable);
    bool PlotDistribution(CTimeSeries<double> *mlr,const QString& variable);
    bool PlotScatter(CMBMatrix *matrix);
    bool PlotScatter(CMBVectorSet *vectorset);
private:
    Ui::GeneralChart *ui;
    Chart* chart;
    ChartView *chartView;
    QComboBox *element_combo;
    QComboBox *independent_combo;
    bool PlotRegression(MultipleLinearRegression *timeseriesset,const QString& independent_var);
    ResultItem* result_item=nullptr;
private slots:
    void onIndependentChanged(int i_independent);
    void onElementChanged(int i_constituent);
    void onPairChanged(int pair_id);
    void onMCMCVariableChanged(int);
    void onDistributionsVariableChanged(int);
    void on_Exporttopng();

};

double roundDown(double a, double rounding_number=1.0);
#endif // GENERALCHART_H
