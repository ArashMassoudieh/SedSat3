#include "cmbtimeseries.h"
#include "QJsonArray"
#include "Vector.h"

CMBTimeSeries::CMBTimeSeries():CTimeSeries<double>(),Interface()
{

}

CMBTimeSeries::CMBTimeSeries(const CMBTimeSeries& mp):CTimeSeries<double>(mp)
{

}

CMBTimeSeries::CMBTimeSeries(int n):CTimeSeries<double>(n), Interface()
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
    for (int i=0; i<n; i++)
    {
        t_values.append(GetT(i));
        C_values.append(GetC(i));
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
    for (int i=0; i<n; i++)
    {
        out+=QString::number(GetT(i)).toStdString() + "," + QString::number(GetC(i)).toStdString();
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
    tablewidget->setRowCount(n);
    QStringList headers;
    QStringList rowlabels;

    for (int i=0; i<n; i++)
    {
        rowlabels<<QString::number(GetT(i));
        tablewidget->setItem(i,0, new QTableWidgetItem(QString::number(GetC(i))));
    }
    headers << "Value";
    tablewidget->setHorizontalHeaderLabels(headers);
    tablewidget->setVerticalHeaderLabels(rowlabels);
    return tablewidget;
}


