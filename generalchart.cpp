#include "generalchart.h"
#include "ui_generalchart.h"
#include <QtCharts>
#include "resultitem.h"
#include <string>


GeneralChart::GeneralChart(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GeneralChart)
{
    ui->setupUi(this);
    chart = new QChart();
    chartView = new QChartView(chart);
    ui->verticalLayout->addWidget(chartView);

}

GeneralChart::~GeneralChart()
{
    delete ui;
}

bool GeneralChart::Plot(ResultItem* res)
{
    result_item = res;
    if (res->Type() == result_type::contribution)
    {

        Contribution* contributions = static_cast<Contribution*>(res->Result());
        return PlotContribution(contributions, QString::fromStdString(res->Name()));

    }
    if (res->Type() == result_type::elemental_profile_set)
    {
        
        Elemental_Profile_Set* profile_sets = static_cast<Elemental_Profile_Set*>(res->Result());
        return PlotProfileSet(profile_sets,QString::fromStdString(res->Name()));

    }

    if (res->Type() == result_type::predicted_concentration)
    {

        Elemental_Profile* profile_set = static_cast<Elemental_Profile*>(res->Result());
        return PlotPredictedConcentration(profile_set,QString::fromStdString(res->Name()));

    }

    if (res->Type() == result_type::vector)
    {
        CMBVector* profile = static_cast<CMBVector*>(res->Result());
        return PlotVector(profile, QString::fromStdString(res->Name()));
    }
    if (res->Type() == result_type::mlrset)
    {
        MultipleLinearRegressionSet* mlrset = static_cast<MultipleLinearRegressionSet*>(res->Result());
        return PlotRegressionSet(mlrset, QString::fromStdString(res->Name()));
    }
    if (res->Type() == result_type::timeseries_set)
    {
        CMBTimeSeriesSet* timeseriesset = static_cast<CMBTimeSeriesSet*>(res->Result());
        return PlotTimeSeriesSet(timeseriesset, QString::fromStdString(res->Name()),QString::fromStdString(res->XAxisTitle()),QString::fromStdString(res->YAxisTitle()));
    }
    if (res->Type() == result_type::rangeset || res->Type() == result_type::rangeset_with_observed)
    {
        RangeSet* rangeset = static_cast<RangeSet*>(res->Result());
        return PlotRangeSet(rangeset, QString::fromStdString(res->Name()),QString::fromStdString(res->XAxisTitle()),QString::fromStdString(res->YAxisTitle()));
    }

    if (res->Type() == result_type::timeseries_set_first_symbol)
    {
        CMBTimeSeriesSet* timeseriesset = static_cast<CMBTimeSeriesSet*>(res->Result());
        return PlotTimeSeriesSet_M(timeseriesset, QString::fromStdString(res->Name()),QString::fromStdString(res->XAxisTitle()),QString::fromStdString(res->YAxisTitle()));
    }

    if (res->Type() == result_type::mcmc_samples)
    {
        CMBTimeSeriesSet* timeseriesset = static_cast<CMBTimeSeriesSet*>(res->Result());
        return InitializeMCMCSamples(timeseriesset, QString::fromStdString(res->Name()));
    }
    if (res->Type() == result_type::distribution || res->Type() == result_type::distribution_with_observed)
    {
        CMBTimeSeriesSet* timeseriesset = static_cast<CMBTimeSeriesSet*>(res->Result());
        return InitializeDistributions(timeseriesset, QString::fromStdString(res->Name()));
    }

    chartView->update();
    return false; 
}

double roundDown(double a, double rounding_value) {
    if (a >= 0)
        return round(a/rounding_value)*rounding_value;
    else
        return -round(-a/rounding_value+1)*rounding_value;

}

bool GeneralChart::PlotVector(CMBVector *profile, const QString &title)
{
    QStringList categories;

    QBarCategoryAxis *axisX = new QBarCategoryAxis();

    vector<string> element_names = profile->Labels();
    for (unsigned int i = 0; i < element_names.size(); i++)
        categories << QString::fromStdString(element_names[i]);

    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);

    QLogValueAxis* axisYLog;
    QValueAxis* axisYNormal;
    bool _log = (result_item->YAxisMode()==yaxis_mode::log?true:false);
    if (profile->min()>0 && _log)
    {
        axisYLog = new QLogValueAxis();
        axisYLog->setRange(pow(10, roundDown(log(profile->min())/log(10.0))), pow(10, int(log(profile->max()) / log(10.0))+1));
        axisYLog->setLabelFormat("%g");
        axisYLog->setMinorTickCount(5);
        chart->addAxis(axisYLog, Qt::AlignLeft);
        _log = true;
    }
    else
    {
        axisYNormal = new QValueAxis();
        axisYNormal->setRange(roundDown(profile->min()*10.0)/10, roundDown((profile->max()+0.1)*10.0)/10);
        axisYNormal->setLabelFormat("%f");
        axisYNormal->setMinorTickCount(5);
        chart->addAxis(axisYNormal, Qt::AlignLeft);
    }

    chart->addAxis(axisX, Qt::AlignBottom);


    //QLineSeries* series = new QLineSeries();
    QBarSeries* series = new QBarSeries();

    chart->addSeries(series);
    series->setName(title);
    series->attachAxis(axisX);
    if (_log)
        series->attachAxis(axisYLog);
    else
        series->attachAxis(axisYNormal);

    double counter = 0.5;
    QBarSet *barset = new QBarSet(QString::fromStdString(result_item->Name()));
    QPen pen;
    pen.setWidth(2);
    barset->setPen(pen);

    for (int i=0; i<profile->num; i++)
    {
        barset->append(profile->at(i));
        counter++;
    }
    series->append(barset);
    return true;

}
bool GeneralChart::PlotProfileSet(Elemental_Profile_Set *profile_sets, const QString &title)
{
    QCategoryAxis* axisX = new QCategoryAxis();

    axisX->setRange(0, profile_sets->ElementNames().size());
    vector<string> element_names = profile_sets->ElementNames();
    for (int i=0; i<element_names.size(); i++)
        axisX->append(QString::fromStdString(element_names[i]),double(i+1));


    QLogValueAxis* axisYLog;
    QValueAxis* axisYNormal;
    bool _log = false;
    if (profile_sets->min()>0)
    {
        axisYLog = new QLogValueAxis();
        axisYLog->setRange(pow(10, roundDown(log(profile_sets->min())/log(10.0))), pow(10, int(log(profile_sets->max()) / log(10.0))+1));
        axisYLog->setLabelFormat("%g");
        axisYLog->setMinorTickCount(5);
        chart->addAxis(axisYLog, Qt::AlignLeft);
        _log = true;
    }
    else
    {
        axisYNormal = new QValueAxis();
        axisYNormal->setRange(roundDown(profile_sets->min()), roundDown(profile_sets->max()+1));
        axisYNormal->setLabelFormat("%g");
        axisYNormal->setMinorTickCount(5);
        chart->addAxis(axisYNormal, Qt::AlignLeft);
    }

    chart->addAxis(axisX, Qt::AlignBottom);


    for (map<string,Elemental_Profile>::iterator it = profile_sets->begin(); it!=profile_sets->end(); it++)
    {
        QScatterSeries* series = new QScatterSeries();
        chart->addSeries(series);
        series->setName(QString::fromStdString(it->first));
        series->attachAxis(axisX);
        if (_log)
            series->attachAxis(axisYLog);
        else
            series->attachAxis(axisYNormal);
        series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
        series->setMarkerSize(15.0);
        double counter=0.5;
        for (map<string, double>::iterator datapoint = it->second.begin(); datapoint != it->second.end(); datapoint++)
        {
            series->append(counter,datapoint->second);
            counter++;
        }

    }
    return true;

}
bool GeneralChart::PlotContribution(Contribution* contributions, const QString &title)
{
    QPieSeries* series = new QPieSeries();
    for (map<string, double>::iterator it = contributions->begin(); it != contributions->end(); it++)
    {
        series->append(QString::fromStdString(it->first), it->second * 100);
    }
    chart->addSeries(series);
    chart->setTitle(title);

    chartView->setRenderHint(QPainter::Antialiasing);
    return true; 
}
bool GeneralChart::PlotPredictedConcentration(Elemental_Profile* profile_set, const QString &title)
{
    QCategoryAxis* axisX = new QCategoryAxis();
    qDebug() << profile_set->ElementNames().size();
    axisX->setRange(0, profile_set->size()*10);
    vector<string> element_names = profile_set->ElementNames();
    for (int i = 0; i < element_names.size(); i++)
        axisX->append(QString::fromStdString(element_names[i]), double(i + 1)*10);


    QLogValueAxis* axisYLog;
    QValueAxis* axisYNormal;
    bool _log = false;
    if (profile_set->min()>0)
    {
        axisYLog = new QLogValueAxis();
        axisYLog->setRange(pow(10, roundDown(log(profile_set->min())/log(10.0))), pow(10, int(log(profile_set->max()) / log(10.0))+1));
        axisYLog->setLabelFormat("%g");
        axisYLog->setMinorTickCount(5);
        chart->addAxis(axisYLog, Qt::AlignLeft);
        _log = true;
    }
    else
    {
        axisYNormal = new QValueAxis();
        axisYNormal->setRange(roundDown(profile_set->min()), roundDown(profile_set->max()+1));
        axisYNormal->setLabelFormat("%f");
        axisYNormal->setMinorTickCount(5);
        chart->addAxis(axisYNormal, Qt::AlignLeft);
    }

    chart->addAxis(axisX, Qt::AlignBottom);


    //QLineSeries* series = new QLineSeries();
    QScatterSeries* series = new QScatterSeries();
    chart->addSeries(series);
    series->setName(title);
    series->attachAxis(axisX);
    if (_log)
        series->attachAxis(axisYLog);
    else
        series->attachAxis(axisYNormal);
    series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    series->setMarkerSize(15.0);
    double counter = 0.5;
    for (map<string, double>::iterator datapoint = profile_set->begin(); datapoint != profile_set->end(); datapoint++)
    {
        series->append(counter*10, datapoint->second);
        counter++;
    }
    return true;


}
bool GeneralChart::PlotRegressionSet(MultipleLinearRegressionSet *regressionset, const QString &title)
{
    element_combo = new QComboBox();
    QLabel *element_label = new QLabel();
    element_label->setText("Constituent:");
    independent_combo = new QComboBox();

    connect(element_combo, SIGNAL(currentIndexChanged(int)),this, SLOT(onElementChanged(int)));
    connect(independent_combo, SIGNAL(currentIndexChanged(int)),this, SLOT(onIndependentChanged(int)));
    for (map<std::string,MultipleLinearRegression>::iterator it = regressionset->begin(); it!=regressionset->end(); it++ )
    {
        element_combo->addItem(QString::fromStdString(it->first));
    }

    QLabel *independent_label = new QLabel();
    independent_label->setText("Independent Variable:");
    ui->horizontalLayout->addWidget(element_label);
    ui->horizontalLayout->addWidget(element_combo);
    ui->horizontalLayout->addWidget(independent_label);
    ui->horizontalLayout->addWidget(independent_combo);
    onElementChanged(element_combo->currentIndex());
    onIndependentChanged(independent_combo->currentIndex());
    return true;
}

bool GeneralChart:: InitializeMCMCSamples(CMBTimeSeriesSet *mcmcsamples, const QString &title)
{
    element_combo = new QComboBox();
    QLabel *element_label = new QLabel();
    element_label->setText("Variable:");
    ui->horizontalLayout->addWidget(element_label);
    ui->horizontalLayout->addWidget(element_combo);

    for (int i = 0; i<mcmcsamples->nvars; i++ )
    {
        element_combo->addItem(QString::fromStdString(mcmcsamples->names[i]));
    }
    connect(element_combo, SIGNAL(currentIndexChanged(int)),this, SLOT(onMCMCVariableChanged(int)));


    onMCMCVariableChanged(element_combo->currentIndex());

    return true;
}

bool GeneralChart::InitializeDistributions(CMBTimeSeriesSet *distributions, const QString &title)
{
    element_combo = new QComboBox();
    QLabel *element_label = new QLabel();
    element_label->setText("Variable:");
    ui->horizontalLayout->addWidget(element_label);
    ui->horizontalLayout->addWidget(element_combo);

    for (int i = 0; i<distributions->nvars; i++ )
    {
        element_combo->addItem(QString::fromStdString(distributions->names[i]));
    }
    connect(element_combo, SIGNAL(currentIndexChanged(int)),this, SLOT(onDistributionsVariableChanged(int)));


    onDistributionsVariableChanged(element_combo->currentIndex());

    return true;
}


void GeneralChart::onElementChanged(int i_element)
{
    MultipleLinearRegressionSet* mlrset = static_cast<MultipleLinearRegressionSet*>(result_item->Result());
    independent_combo->clear();
    QString constituent = element_combo->itemText(i_element);
    for (unsigned int i=0; i<mlrset->at(constituent.toStdString()).GetIndependentVariableNames().size(); i++)
    {
        independent_combo->addItem(QString::fromStdString(mlrset->at(constituent.toStdString()).GetIndependentVariableNames()[i]));
    }
    if (!independent_combo->currentText().isEmpty() && !element_combo->currentText().isEmpty())
    {
        PlotRegression(&mlrset->at(constituent.toStdString()),independent_combo->currentText());
    }

}

void GeneralChart::onMCMCVariableChanged(int i)
{
    CMBTimeSeriesSet * samplesset = static_cast<CMBTimeSeriesSet*>(result_item->Result());
    QString variable = element_combo->itemText(i);
    PlotMCMCSamples(&samplesset->operator[](variable.toStdString()),variable);
}

void GeneralChart::onDistributionsVariableChanged(int i)
{
    CMBTimeSeriesSet * samplesset = static_cast<CMBTimeSeriesSet*>(result_item->Result());
    QString variable = element_combo->itemText(i);
    PlotDistribution(&samplesset->operator[](variable.toStdString()),variable);
}
void GeneralChart::onIndependentChanged(int i_constituent)
{
    MultipleLinearRegressionSet* mlrset = static_cast<MultipleLinearRegressionSet*>(result_item->Result());
    if (!independent_combo->currentText().isEmpty() && !element_combo->currentText().isEmpty())
    {
        PlotRegression(&mlrset->at(element_combo->currentText().toStdString()),independent_combo->currentText());
    }

}

bool GeneralChart::PlotRegression(MultipleLinearRegression *mlr,const QString& independent_var)
{

    for (int i=0; i<chart->axes(Qt::Horizontal).size(); i++)
    {
        qDebug()<<chart->axes(Qt::Horizontal)[i]->objectName();
    }

    for (int i=0; i<chart->axes(Qt::Vertical).size(); i++)
    {
        qDebug()<<chart->axes(Qt::Vertical)[i]->objectName();
    }

    chart->removeAllSeries();
    for (int i=0; i<chart->axes().size(); i++)
    {
        chart->removeAxis(chart->axes()[i]);
        //delete chart->axes()[i];
    }

    qDebug()<<"After:";
    for (int i=0; i<chart->axes(Qt::Horizontal).size(); i++)
    {
        qDebug()<<chart->axes(Qt::Horizontal)[i]->objectName();
        chart->removeAxis(chart->axes(Qt::Horizontal)[i]);
    }

    for (int i=0; i<chart->axes(Qt::Vertical).size(); i++)
    {
        qDebug()<<chart->axes(Qt::Vertical)[i]->objectName();
        chart->removeAxis(chart->axes(Qt::Vertical)[i]);
    }
    chart->axes().clear();
    QValueAxis* axisX = new QValueAxis();
    QValueAxis* axisYNormal = new QValueAxis();
    axisX->setObjectName("axisX");
    axisYNormal->setObjectName("axisY");

    QLineSeries *lineseries = new QLineSeries();
    double x_min_val = CVector(mlr->IndependentData(independent_var.toStdString())).min();
    double x_max_val = CVector(mlr->IndependentData(independent_var.toStdString())).max();
    double y_min_val = mlr->CoefficientsIntercept()[0];
    double y_max_val = mlr->CoefficientsIntercept()[0];
    for (int i=0; i<mlr->GetIndependentVariableNames().size(); i++)
    {
        if (mlr->GetIndependentVariableNames()[i]!=independent_var.toStdString())
        {
            y_min_val += mlr->MeanIndependentVar(i)*mlr->CoefficientsIntercept()[i+1];
            y_max_val += mlr->MeanIndependentVar(i)*mlr->CoefficientsIntercept()[i+1];
        }
        else
        {
            y_min_val += x_min_val*mlr->CoefficientsIntercept()[i+1];
            y_max_val += x_max_val*mlr->CoefficientsIntercept()[i+1];
        }
    }

    axisX->setRange(CVector(mlr->IndependentData(independent_var.toStdString())).min(), CVector(mlr->IndependentData(independent_var.toStdString())).max());
    axisX->setTitleText(independent_var);

    //axisYNormal->setLabelFormat("%f");
    //axisYNormal->setMinorTickCount(5);
    axisYNormal->setTitleText(QString::fromStdString("Measured " + mlr->DependentVariableName()));


    QScatterSeries* series = new QScatterSeries();

    series->setName(QString::fromStdString(mlr->DependentVariableName()));
    series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    series->setMarkerSize(15.0);
    qDebug()<<independent_var;
    for (unsigned int i=0; i<mlr->IndependentData(independent_var.toStdString()).size(); i++)
    {
        qDebug()<<mlr->DependentData()[i] << ","<< mlr->IndependentData(independent_var.toStdString())[i];
        series->append(mlr->IndependentData(independent_var.toStdString())[i],mlr->DependentData()[i]);
    }

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisYNormal, Qt::AlignLeft);
    chart->addSeries(series);
    series->attachAxis(axisX);
    series->attachAxis(axisYNormal);


    lineseries->append(x_min_val,y_min_val);
    lineseries->append(x_max_val,y_max_val);
    chart->addSeries(lineseries);
    lineseries->attachAxis(axisX);
    lineseries->attachAxis(axisYNormal);
    lineseries->setName("Regression");
    axisYNormal->setRange(std::min(std::min(CVector(mlr->DependentData()).min(),y_min_val),y_max_val),std::max(std::max(CVector(mlr->DependentData()).max(),y_max_val),y_min_val));

    return true;
}


bool GeneralChart::PlotMCMCSamples(CTimeSeries<double> *samples,const QString& variable)
{

    chart->removeAllSeries();
    for (int i=0; i<chart->axes().size(); i++)
    {
        chart->removeAxis(chart->axes()[i]);
        //delete chart->axes()[i];
    }

    for (int i=0; i<chart->axes(Qt::Horizontal).size(); i++)
    {
        chart->removeAxis(chart->axes(Qt::Horizontal)[i]);
    }

    for (int i=0; i<chart->axes(Qt::Vertical).size(); i++)
    {
        chart->removeAxis(chart->axes(Qt::Vertical)[i]);
    }
    chart->axes().clear();
    QValueAxis* axisX = new QValueAxis();
    QValueAxis* axisYNormal = new QValueAxis();
    axisX->setObjectName("axisX");
    axisYNormal->setObjectName("axisY");

    double x_min_val = samples->mint();
    double x_max_val = samples->maxt();
    double y_min_val = samples->minC();
    double y_max_val = samples->maxC();


    axisX->setRange(x_min_val, x_max_val);
    axisX->setTitleText("Sample number");

    //axisYNormal->setLabelFormat("%f");
    //axisYNormal->setMinorTickCount(5);
    axisYNormal->setTitleText(variable);
    axisYNormal->setRange(y_min_val, y_max_val);


    QScatterSeries* series = new QScatterSeries();

    QPen marker_pen(QBrush(Qt::SolidPattern),1,Qt::PenStyle::SolidLine);

    series->setName(variable);
    series->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
    series->setMarkerSize(3.0);
    series->setPen(marker_pen);


    for (int i=0; i<samples->n; i++)
    {
        series->append(samples->GetT(i),samples->GetC(i));
    }

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisYNormal, Qt::AlignLeft);
    chart->addSeries(series);
    series->attachAxis(axisX);
    series->attachAxis(axisYNormal);

    return true;
}


bool GeneralChart::PlotDistribution(CTimeSeries<double> *samples,const QString& variable)
{
    chart->removeAllSeries();
    for (int i=0; i<chart->axes().size(); i++)
    {
        chart->removeAxis(chart->axes()[i]);
        //delete chart->axes()[i];
    }

    for (int i=0; i<chart->axes(Qt::Horizontal).size(); i++)
    {
        chart->removeAxis(chart->axes(Qt::Horizontal)[i]);
    }

    for (int i=0; i<chart->axes(Qt::Vertical).size(); i++)
    {
        chart->removeAxis(chart->axes(Qt::Vertical)[i]);
    }
    chart->axes().clear();
    QValueAxis* axisX = new QValueAxis();
    QValueAxis* axisYNormal = new QValueAxis();
    axisX->setObjectName("axisX");
    axisYNormal->setObjectName("axisY");

    double x_min_val = samples->mint();
    double x_max_val = samples->maxt();
    double y_min_val = samples->minC();
    double y_max_val = samples->maxC();


    axisX->setRange(x_min_val, x_max_val);
    axisX->setTitleText("Sample number");
    axisYNormal->setTitleText(variable);
    axisYNormal->setRange(y_min_val, y_max_val);

    QLineSeries *series = new QLineSeries();
    QAreaSeries *areaseries = new QAreaSeries(series);

    QPen marker_pen(QBrush(Qt::SolidPattern),1,Qt::PenStyle::SolidLine);

    series->setName(variable);
    series->setPen(marker_pen);

    for (int i=0; i<samples->n; i++)
    {
        series-> append(samples->GetT(i),samples->GetC(i));
    }

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisYNormal, Qt::AlignLeft);

    chart->addSeries(series);
    series->attachAxis(axisX);
    series->attachAxis(axisYNormal);

    chart->addSeries(areaseries);
    areaseries->attachAxis(axisX);
    areaseries->attachAxis(axisYNormal);

    if (result_item->Type() == result_type::distribution_with_observed)
    {
        QLineSeries *observed_series = new QLineSeries;
        QPen observed_pen(QBrush(Qt::DashLine),3,Qt::PenStyle::DashLine);
        observed_pen.setColor(Qt::black);
        observed_series->setPen(observed_pen);
        observed_series->append(static_cast<CMBTimeSeriesSet*>(result_item->Result())->ObservedValue(variable.toStdString()),0);
        observed_series->append(static_cast<CMBTimeSeriesSet*>(result_item->Result())->ObservedValue(variable.toStdString()),y_max_val);
        chart->addSeries(observed_series);
        observed_series->attachAxis(axisX);
        observed_series->attachAxis(axisYNormal);
    }
    return true;
}

bool GeneralChart::PlotTimeSeriesSet(CMBTimeSeriesSet *timeseriesset, const QString &title, const QString &x_axis_title, const QString &y_axis_title)
{
    double x_min_val = timeseriesset->mintime();
    double x_max_val = timeseriesset->maxtime();
    double y_min_val = timeseriesset->minval();
    double y_max_val = timeseriesset->maxval();
    QString xAxisTitle = x_axis_title;
    QString yAxisTitle = y_axis_title;
    if (x_axis_title.isEmpty()) xAxisTitle = "Value";
    if (y_axis_title.isEmpty()) yAxisTitle = "CDF";
    QValueAxis* axisX = new QValueAxis();
    QValueAxis* axisY = new QValueAxis();
    axisX->setObjectName("axisX");
    axisY->setObjectName("axisY");
    axisX->setTitleText(xAxisTitle);
    axisY->setTitleText(yAxisTitle);
    axisX->setRange(x_min_val,x_max_val);
    axisY->setRange(y_min_val,y_max_val);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    for (int i=0; i<timeseriesset->nvars; i++)
    {
        QLineSeries *lineseries = new QLineSeries();
        chart->addSeries(lineseries);
        lineseries->attachAxis(axisX);
        lineseries->attachAxis(axisY);

        for (int j=0; j<timeseriesset->BTC[i].n; j++)
        {
            lineseries->append(timeseriesset->BTC[i].GetT(j),timeseriesset->BTC[i].GetC(j));
        }
        QPen pen = lineseries->pen();
        pen.setWidth(2);
        pen.setBrush(QColor(QRandomGenerator::global()->bounded(256), QRandomGenerator::global()->bounded(256), QRandomGenerator::global()->bounded(256)));
        lineseries->setPen(pen);
        lineseries->setName(QString::fromStdString(timeseriesset->names[i]));

    }

    return true;
}

bool GeneralChart::PlotRangeSet(RangeSet *rangeset, const QString &title, const QString &x_axis_title, const QString &y_axis_title)
{
    double y_min_val = rangeset->minval();
    double y_max_val = rangeset->maxval();

    if (result_item->FixedYLimit() && result_item->YLimit(_range::high)!=0)
    {
        y_max_val = result_item->YLimit(_range::high);
    }

    if (result_item->FixedYLimit() && result_item->YLimit(_range::low)!=0)
    {
        y_min_val = result_item->YLimit(_range::low);
    }

    if (result_item->YAxisMode()==yaxis_mode::log)
    {
        y_min_val = max(y_min_val,1e-8);
    }
    QString xAxisTitle = x_axis_title;
    QString yAxisTitle = y_axis_title;
    if (x_axis_title.isEmpty()) xAxisTitle = "Constituent";
    if (y_axis_title.isEmpty()) yAxisTitle = "95% CI";
    QCategoryAxis* axisX = new QCategoryAxis();

    QLogValueAxis* axisYLog = new QLogValueAxis();
    QValueAxis* axisY = new QValueAxis();

    axisX->setObjectName("axisX");
    axisY->setObjectName("axisY");
    axisYLog->setObjectName("axisYLog");
    axisYLog->setMinorTickCount(10);

    axisX->setTitleText(xAxisTitle);
    if (result_item->YAxisMode()==yaxis_mode::log)
    {   axisYLog->setTitleText(yAxisTitle);
        axisYLog->setRange(y_min_val,y_max_val);
        axisYLog->setLabelFormat("%g");
    }
    else
    {   axisY->setTitleText(yAxisTitle);
        axisY->setRange(y_min_val,y_max_val);
        axisYLog->setLabelFormat("%f");
    }
    axisX->setRange(-0.5,rangeset->size()-0.5);


    chart->addAxis(axisX, Qt::AlignBottom);
    if (result_item->YAxisMode()==yaxis_mode::log)
        chart->addAxis(axisYLog, Qt::AlignLeft);
    else
        chart->addAxis(axisY, Qt::AlignLeft);

    double upperlimit = 0.5;
    axisX->setStartValue(-0.5);
    QBoxPlotSeries *boxWhiskSeries = new QBoxPlotSeries();

    for (map<string, Range>::iterator it = rangeset->begin(); it!=rangeset->end(); it++)
    {
        axisX->append(QString::fromStdString(it->first),upperlimit);
        upperlimit += 1;
    }

    if (result_item->Type() == result_type::rangeset_with_observed)
    {   QScatterSeries* scatterseries = new QScatterSeries();
        chart->addSeries(scatterseries);
        scatterseries->attachAxis(axisX);
        if (result_item->YAxisMode()==yaxis_mode::log)
            scatterseries->attachAxis(axisYLog);
        else
            scatterseries->attachAxis(axisY);
        int counter = 0;
        for (map<string, Range>::iterator it = rangeset->begin(); it!=rangeset->end(); it++)
        {
            scatterseries->append(counter,it->second.GetValue());
            counter++;
        }
        QPen pen = scatterseries->pen();
        pen.setWidth(2);

        pen.setBrush(QColor(QRandomGenerator::global()->bounded(256), QRandomGenerator::global()->bounded(256), QRandomGenerator::global()->bounded(256)));
        scatterseries->setPen(pen);
        scatterseries->setName(QString::fromStdString("Observed value"));
    }

    for (map<string, Range>::iterator it = rangeset->begin(); it!=rangeset->end(); it++)
    {
        QBoxSet *boxSet = new QBoxSet();
        qDebug()<<it->second.Get(_range::low)<<","<<it->second.Get(_range::high)<<","<<it->second.GetValue();
        boxSet->setValue(QBoxSet::LowerExtreme, max(it->second.Get(_range::low),y_min_val));
        boxSet->setValue(QBoxSet::UpperExtreme, it->second.Get(_range::high));
        boxSet->setValue(QBoxSet::LowerQuartile, max(it->second.Get(_range::low),y_min_val));
        boxSet->setValue(QBoxSet::UpperQuartile, it->second.Get(_range::high));
        boxSet->setValue(QBoxSet::Median, it->second.Median());
        boxWhiskSeries->append(boxSet);


    }
    QPen brushcolor(QColor(0,200,0,126));
    QBrush brush(QColor(0,200,0,126));
    boxWhiskSeries->setBrush(brush);
    boxWhiskSeries->setName("95% Credible Interval");

    chart->addSeries(boxWhiskSeries);
    if (result_item->YAxisMode()==yaxis_mode::log)
        boxWhiskSeries->attachAxis(axisYLog);
    else
        boxWhiskSeries->attachAxis(axisY);
    boxWhiskSeries->attachAxis(axisX);

    return true;
}

bool GeneralChart::PlotTimeSeriesSet_M(CMBTimeSeriesSet *timeseriesset, const QString &title, const QString &x_axis_title, const QString &y_axis_title)
{
    double x_min_val = timeseriesset->mintime();
    double x_max_val = timeseriesset->maxtime();
    double y_min_val = timeseriesset->minval();
    double y_max_val = timeseriesset->maxval();
    QValueAxis* axisX = new QValueAxis();
    QValueAxis* axisY = new QValueAxis();
    QString xAxisTitle = x_axis_title;
    QString yAxisTitle = y_axis_title;
    if (x_axis_title.isEmpty()) xAxisTitle = "Value";
    if (y_axis_title.isEmpty()) yAxisTitle = "CDF";
    axisX->setObjectName("axisX");
    axisY->setObjectName("axisY");
    axisX->setTitleText(xAxisTitle);
    axisY->setTitleText(yAxisTitle);
    axisX->setRange(x_min_val,x_max_val);
    axisY->setRange(y_min_val,y_max_val);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    int i=0;
    {
        QScatterSeries* scatterseries = new QScatterSeries();
        chart->addSeries(scatterseries);
        scatterseries->attachAxis(axisX);
        scatterseries->attachAxis(axisY);

        for (int j=0; j<timeseriesset->BTC[i].n; j++)
        {
            scatterseries->append(timeseriesset->BTC[i].GetT(j),timeseriesset->BTC[i].GetC(j));
        }
        QPen pen = scatterseries->pen();
        pen.setWidth(2);

        pen.setBrush(QColor(QRandomGenerator::global()->bounded(256), QRandomGenerator::global()->bounded(256), QRandomGenerator::global()->bounded(256)));
        scatterseries->setPen(pen);
        scatterseries->setName(QString::fromStdString(timeseriesset->names[i]));

    }
    for (int i=1; i<timeseriesset->nvars; i++)
    {
        QLineSeries *lineseries = new QLineSeries();
        chart->addSeries(lineseries);
        lineseries->attachAxis(axisX);
        lineseries->attachAxis(axisY);

        for (int j=0; j<timeseriesset->BTC[i].n; j++)
        {
            lineseries->append(timeseriesset->BTC[i].GetT(j),timeseriesset->BTC[i].GetC(j));
        }
        QPen pen = lineseries->pen();
        pen.setWidth(2);
        pen.setBrush(QColor(QRandomGenerator::global()->bounded(256), QRandomGenerator::global()->bounded(256), QRandomGenerator::global()->bounded(256)));
        lineseries->setPen(pen);
        lineseries->setName(QString::fromStdString(timeseriesset->names[i]));

    }

    return true;
}
