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
    bool _log = false;
    if (profile->min()>0)
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
    QBarSet *barset = new QBarSet("DFA Eigenvector");
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
    QComboBox *element_combo = new QComboBox();
    QLabel *element_label = new QLabel();
    element_label->setText("Constituent:");
    QComboBox *independent_combo = new QComboBox();
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
    return true;
}
