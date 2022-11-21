#include "generalchart.h"
#include "ui_generalchart.h"
#include <QtCharts>
#include <contribution.h>
#include <elemental_profile_set.h>

GeneralChart::GeneralChart(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GeneralChart)
{
    ui->setupUi(this);
    chart = new QtCharts::QChart();
    chartView = new QtCharts::QChartView(chart);
    ui->verticalLayout->addWidget(chartView);

}

GeneralChart::~GeneralChart()
{
    delete ui;
}

bool GeneralChart::Plot(result_item* res)
{
    if (res->type == result_type::contribution)
    {
        QPieSeries* series = new QPieSeries();
        Contribution* contributions = static_cast<Contribution*>(res->result);
        for (map<string, double>::iterator it = contributions->begin(); it != contributions->end(); it++)
        {
            series->append(QString::fromStdString(it->first), it->second * 100);
        }
        chart->addSeries(series);
        chart->setTitle(QString::fromStdString(res->name));
        //chart->legend()->hide();
        chartView->setRenderHint(QPainter::Antialiasing);
        return true;
    }
    if (res->type == result_type::elemental_profile_set)
    {
        
        Elemental_Profile_Set* profile_sets = static_cast<Elemental_Profile_Set*>(res->result);
        
        QCategoryAxis* axisX = new QCategoryAxis();
        
        axisX->setRange(0, profile_sets->ElementNames().size());
        vector<string> element_names = profile_sets->ElementNames(); 
        for (int i=0; i<element_names.size(); i++)
            axisX->append(QString::fromStdString(element_names[i]),double(i+1));
        

        QLogValueAxis* axisY = new QLogValueAxis();
        qDebug() << profile_sets->min()<<":"<< pow(10, roundDown(log(profile_sets->min()) / log(10.0)));
        axisY->setRange(pow(10, roundDown(log(profile_sets->min())/log(10.0))), pow(10, int(log(profile_sets->max()) / log(10.0))+1));
        axisY->setLabelFormat("%g");
        axisY->setMinorTickCount(5);
        
        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY, Qt::AlignLeft);
                
        for (map<string,Elemental_Profile>::iterator it = profile_sets->begin(); it!=profile_sets->end(); it++)
        { 
            QScatterSeries* series = new QScatterSeries();
            chart->addSeries(series);
            series->setName(QString::fromStdString(it->first));
            series->attachAxis(axisX);
            series->attachAxis(axisY);
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

    if (res->type == result_type::predicted_concentration)
    {

        Elemental_Profile* profile_set = static_cast<Elemental_Profile*>(res->result);

        QCategoryAxis* axisX = new QCategoryAxis();
        qDebug() << profile_set->ElementNames().size();
        axisX->setRange(0, profile_set->size()*10);
        vector<string> element_names = profile_set->ElementNames();
        for (int i = 0; i < element_names.size(); i++)
            axisX->append(QString::fromStdString(element_names[i]), double(i + 1)*10);


        QLogValueAxis* axisY = new QLogValueAxis();
        axisY->setRange(pow(10, roundDown(log(profile_set->min()) / log(10.0))), pow(10, int(log(profile_set->max()) / log(10.0)) + 1));
        axisY->setLabelFormat("%g");
        axisY->setMinorTickCount(5);

        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY, Qt::AlignLeft);

        //QLineSeries* series = new QLineSeries();
        QScatterSeries* series = new QScatterSeries();
        chart->addSeries(series);
        series->setName(QString::fromStdString(res->name));
        series->attachAxis(axisX);
        series->attachAxis(axisY);
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

double roundDown(double a) {
    if (a >= 0)
        return round(a);
    else
        return -round(-a+1);

}
