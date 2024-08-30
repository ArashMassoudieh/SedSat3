#include "cmbvectorsetset.h"
#include <QFile>
#include <gsl/gsl_cdf.h>

CMBVectorSetSet::CMBVectorSetSet():Interface(),map<string, CMBVectorSet>()
{

}
CMBVectorSetSet::CMBVectorSetSet(const CMBVectorSetSet& mp):Interface(mp),map<string,CMBVectorSet>(mp)
{

}
CMBVectorSetSet& CMBVectorSetSet::operator=(const CMBVectorSetSet &mp)
{
    map<string,CMBVectorSet>::operator=(mp);
    Interface::operator=(mp);
    return *this;
}
QJsonObject CMBVectorSetSet::toJsonObject()
{
    QJsonObject out;
    for (map<string,CMBVectorSet>::iterator it = begin(); it!=end();it++)
    {
        out[QString::fromStdString(it->first)]=it->second.toJsonObject();
    }
    return out;
}
bool CMBVectorSetSet::ReadFromJsonObject(const QJsonObject &jsonobject)
{
    for(QString key: jsonobject.keys() ) {
        CMBVectorSet column;
        column.ReadFromJsonObject(jsonobject[key].toObject());
        operator[](key.toStdString()) = column;
    }
    return true;
}
string CMBVectorSetSet::ToString()
{
    string s;
    for (map<string,CMBVectorSet>::iterator it = begin(); it!=end();it++)
    {
        s+= "\n";
        s+=it->first + ":\n";
        s+=it->second.ToString()+="\n";
    }
    return s;
}

unsigned int CMBVectorSetSet::MaxSize() const
{
    int max_size=0;
    for (map<string,CMBVectorSet>::const_iterator it = begin(); it!=end();it++)
    {
        if (it->second.MaxSize()>max_size)
            max_size = it->second.MaxSize();
    }
    return max_size;
}

double CMBVectorSetSet::max() const
{
    double out = -1e23;
    for (map<string,CMBVectorSet>::const_iterator it = cbegin(); it!=cend();it++)
    {
        out = std::max(out,it->second.max());
    }
    return out;
};
double CMBVectorSetSet::min() const
{
    double out = 1e23;
    for (map<string,CMBVectorSet>::const_iterator it = cbegin(); it!=cend();it++)
    {
        out = std::min(out,it->second.min());
    }
    return out;
}


QTableWidget *CMBVectorSetSet::ToTable()
{
    QTableWidget *tablewidget = new QTableWidget();
    tablewidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    int column_count=0;
    for (map<string,CMBVectorSet>::iterator vectorset=begin(); vectorset!=end(); vectorset++)
            column_count += vectorset->second.size();
    tablewidget->setColumnCount(column_count*2);
    tablewidget->setRowCount(MaxSize());
    QStringList rowheaders;
    QStringList colheaders;
    int colno = 0;
    for (map<string,CMBVectorSet>::iterator vectorset=begin(); vectorset!=end(); vectorset++)
    {
        for (map<string,CMBVector>::iterator column=vectorset->second.begin(); column!=vectorset->second.end(); column++)
        {
            colheaders<<QString::fromStdString(vectorset->first)+":"+QString::fromStdString(column->first)<<QString::fromStdString(vectorset->first)+":"+QString::fromStdString(column->first)+"-Value";
            for (int i=0; i<column->second.getsize(); i++)
            {
                tablewidget->setItem(i,colno*2,new QTableWidgetItem(QString::fromStdString(column->second.Label(i))));
                tablewidget->setItem(i,colno*2+1,new QTableWidgetItem(QString::number(column->second[i])));
            }
            colno++;
        }
    }

    tablewidget->setHorizontalHeaderLabels(colheaders);
    //tablewidget->setVerticalHeaderLabels(rowheaders);
    return tablewidget;
}
bool CMBVectorSetSet::writetofile(QFile* file)
{
    file->write(QString::fromStdString(ToString()).toUtf8());
    return true;
}

string CMBVectorSetSet::Label(const string &vectorset,const string &column, int j) const
{
    if (count(vectorset)!=0)
        return (at(vectorset).Label(column,j));
    else
        return "";
}
double CMBVectorSetSet::valueAt(const string &vectorset, const string &columnlabel, int j) const
{
    if (count(columnlabel)!=0)
        return (at(vectorset).valueAt(columnlabel,j));
    else
        return 0;
}
CMBVector &CMBVectorSetSet::GetColumn(const string &vectorset, const string &columnlabel)
{
    return at(vectorset).GetColumn(columnlabel);
}
void CMBVectorSetSet::Append(const string &columnlabel,const CMBVectorSet &vectorset)
{
    this->operator[](columnlabel) = vectorset;
}

