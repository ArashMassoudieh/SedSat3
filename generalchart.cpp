#include "generalchart.h"
#include "ui_generalchart.h"
#include <QtCharts>
#include <contribution.h>
#include <elemental_profile_set.h>
#include "resultitem.h"

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
        QPieSeries* series = new QPieSeries();
        Contribution* contributions = static_cast<Contribution*>(res->Result());
        for (map<string, double>::iterator it = contributions->begin(); it != contributions->end(); it++)
        {
            series->append(QString::fromStdString(it->first), it->second * 100);
        }
        chart->addSeries(series);
        chart->setTitle(QString::fromStdString(res->Name()));
        //chart->legend()->hide();
        chartView->setRenderHint(QPainter::Antialiasing);
        return true;
    }
    if (res->Type() == result_type::elemental_profile_set)
    {
        
        Elemental_Profile_Set* profile_sets = static_cast<Elemental_Profile_Set*>(res->Result());
        
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

    if (res->Type() == result_type::predicted_concentration)
    {

        Elemental_Profile* profile_set = static_cast<Elemental_Profile*>(res->Result());

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
        series->setName(QString::fromStdString(res->Name()));
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
        //*series << QPointF(0, 6) << QPointF(9, 4) << QPointF(15, 20) << QPointF(25, 12) << QPointF(29, 26);
        
        

        return true;
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
