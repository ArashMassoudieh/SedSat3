#include "contribution.h"
#include "Utilities.h"
#include "qfile.h"

Contribution::Contribution():map<string, double>(),Interface()
{

}

Contribution::Contribution(const Contribution &rhs):map<string, double>(rhs),Interface()
{

}
Contribution& Contribution::operator = (const Contribution &rhs)
{
    map<string,double>::operator=(rhs);
    Interface::operator=(rhs);
    return *this;
}

string Contribution::ToString() const
{
    string out;
    for (map<string,double>::const_iterator it=cbegin(); it!=cend(); it++ )
    {
        out += it->first + ":" + aquiutils::numbertostring(it->second) + "\n";
    }
    return out;
}

bool Contribution::writetofile(QFile* file)
{
    file->write(QString::fromStdString(ToString()).toUtf8());
    return true;
}

bool Contribution::Read(const QStringList &strlist)
{
    clear();
    for (int i=0; i<strlist.size();i++)
    {
        if (strlist[i].split(":").size()>1)
        {
            operator[](strlist[i].split(":")[0].toStdString()) = strlist[i].split(":")[1].toDouble();
        }
    }
    return true;
}

QJsonObject Contribution::toJsonObject() const
{
    QJsonObject out;
    for (map<string,double>::const_iterator it=cbegin(); it!=cend(); it++ )
    {
        out[QString::fromStdString(it->first)] = it->second;
    }
    return out;
}

bool Contribution::ReadFromJsonObject(const QJsonObject &jsonobject)
{
    clear();
    for (QString key: jsonobject.keys())
    {
        operator[](key.toStdString()) = jsonobject[key].toDouble();
    }
    return true;
}

QTableWidget *Contribution::ToTable()
{
    QTableWidget *tablewidget = new QTableWidget();
    tablewidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablewidget->setColumnCount(1);
    tablewidget->setRowCount(size());
    QStringList headers;
    QStringList sources;
    int i=0;
    for (map<string,double>::const_iterator it=cbegin(); it!=cend(); it++ )
    {
        sources<<QString::fromStdString(it->first);
        tablewidget->setItem(i,0, new QTableWidgetItem(QString::number(it->second)));
        i++;
    }
    headers << "Contribution";
    tablewidget->setHorizontalHeaderLabels(headers);
    tablewidget->setVerticalHeaderLabels(sources);
    return tablewidget;
}
