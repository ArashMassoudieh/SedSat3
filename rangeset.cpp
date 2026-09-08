#include "rangeset.h"
#include <QFile>

RangeSet::RangeSet():map<string, Range>(),Interface()
{

}

RangeSet::RangeSet(const RangeSet &rhs):map<string, Range>(),Interface()
{

}
RangeSet& RangeSet::operator = (const RangeSet &rhs)
{
    Interface::operator=(rhs);
    map<string,Range>::operator=(rhs);
    return *this;
}
QJsonObject RangeSet::toJsonObject() const
{
    QJsonObject out;
    for (map<string,Range>::const_iterator it=cbegin(); it!=cend(); it++)
    {
        out[QString::fromStdString(it->first)] = it->second.toJsonObject();
    }
    return out;
}
bool RangeSet::ReadFromJsonObject(const QJsonObject &jsonobject)
{
    for (QString key: jsonobject.keys())
    {
        Range range;
        range.ReadFromJsonObject(jsonobject[key].toObject());
        this->operator[](key.toStdString()) = range;
    }
    return true;
}
string RangeSet::ToString() const
{
    string out;
    for (map<string,Range>::const_iterator it=cbegin(); it!=cend(); it++)
    {
        out.append(it->first + ":" + it->second.ToString());
    }
    return out;
}

bool RangeSet::writetofile(QFile* file)
{
    file->write(QString::fromStdString(ToString()).toUtf8());
    return true;

}
bool RangeSet::Read(const QStringList &strlist)
{
    return true;
}

double RangeSet::maxval()
{
    double out = -1e24;
    for (map<string,Range>::iterator it=begin(); it!=end(); it++)
    {
        out = max(out,it->second.Get(_range::high));
        if (it->second.GetValue()!=0)
            out = max(out,it->second.GetValue());
    }
    return out;
}
double RangeSet::minval()
{
    double out = 1e24;
    for (map<string,Range>::iterator it=begin(); it!=end(); it++)
    {
        out = min(out,it->second.Get(_range::low));
        if (it->second.GetValue()!=0)
            out = min(out,it->second.GetValue());
    }
    return out;
}

QTableWidget *RangeSet::ToTable()
{
    QTableWidget *tablewidget = new QTableWidget();
    tablewidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablewidget->setColumnCount(4);
    tablewidget->setRowCount(size());
    QStringList headers;
    headers<<"Low"<<"High"<<"Median"<<"Mean";
    QStringList constituents;
    int row = 0;
    for (map<string,Range>::iterator it=begin(); it!=end(); it++ )
    {
        tablewidget->setItem(row,0, new QTableWidgetItem(QString::number(it->second.Get(_range::low))));
        tablewidget->setItem(row,1, new QTableWidgetItem(QString::number(it->second.Get(_range::high))));
        tablewidget->setItem(row,2, new QTableWidgetItem(QString::number(it->second.Median())));
        tablewidget->setItem(row,3, new QTableWidgetItem(QString::number(it->second.Mean())));
        row++;
        constituents << QString::fromStdString(it->first);
    }

    tablewidget->setHorizontalHeaderLabels(headers);
    tablewidget->setVerticalHeaderLabels(constituents);
    return tablewidget;
}
