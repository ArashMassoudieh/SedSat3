#include "cmbtimeseries.h"
#include "QJsonArray"
#include <QFile>
#include "Vector.h"

CMBTimeSeries::CMBTimeSeries():TimeSeries<double>(),Interface()
{

}

CMBTimeSeries::CMBTimeSeries(const CMBTimeSeries& mp):TimeSeries<double>(mp)
{

}

CMBTimeSeries::CMBTimeSeries(int n):TimeSeries<double>(n), Interface()
{

}

CMBTimeSeries& CMBTimeSeries::operator=(const CMBTimeSeries &mp)
{
    CMBTimeSeries::operator=(mp);
    return *this;
}
QJsonObject CMBTimeSeries::toJsonObject()
{
    QJsonObject out;
    QJsonArray t_values;
    QJsonArray C_values;
    for (int i=0; i<size(); i++)
    {
        t_values.append(getTime(i));
        C_values.append(getValue(i));
    }
    out["time"] = t_values;
    out["value"] = C_values;

    return out;

}
bool CMBTimeSeries::ReadFromJsonObject(const QJsonObject &jsonobject)
{
    return true;
}


string CMBTimeSeries::ToString()
{
    string out;
    for (int i=0; i<size(); i++)
    {
        out+=QString::number(getTime(i)).toStdString() + "," + QString::number(getValue(i)).toStdString();
    }

    return out;
}

bool CMBTimeSeries::writetofile(QFile* file)
{
    file->write(QString::fromStdString(ToString()).toUtf8());
    return true;
}

QTableWidget *CMBTimeSeries::ToTable()
{
    QTableWidget *tablewidget = new QTableWidget();
    tablewidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablewidget->setColumnCount(1);
    tablewidget->setRowCount(size());
    QStringList headers;
    QStringList rowlabels;

    for (int i=0; i<size(); i++)
    {
        rowlabels<<QString::number(getTime(i));
        if (highlightoutsideoflimit)
        {
            if (getTime(i)>highlimit || getValue(i)<lowlimit)
            {
                tablewidget->item(i,0)->setForeground(QColor(Qt::red));
            }
        }
        tablewidget->setItem(i,0, new QTableWidgetItem(QString::number(getValue(i))));
    }
    headers << "Value";
    tablewidget->setHorizontalHeaderLabels(headers);
    tablewidget->setVerticalHeaderLabels(rowlabels);
    return tablewidget;
}


