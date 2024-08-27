#include "cmbvectorset.h"
#include <QFile>

CMBVectorSet::CMBVectorSet():Interface(),map<string, CMBVector>()
{

}
CMBVectorSet::CMBVectorSet(const CMBVectorSet& mp):Interface(mp),map<string,CMBVector>(mp)
{

}
CMBVectorSet& CMBVectorSet::operator=(const CMBVectorSet &mp)
{
    map<string,CMBVector>::operator=(mp);
    Interface::operator=(mp);
    return *this;
}
QJsonObject CMBVectorSet::toJsonObject()
{
    QJsonObject out;
    for (map<string,CMBVector>::iterator it = begin(); it!=end();it++)
    {
        out[QString::fromStdString(it->first)]=it->second.toJsonObject();
    }
    return out;
}
bool CMBVectorSet::ReadFromJsonObject(const QJsonObject &jsonobject)
{
    for(QString key: jsonobject.keys() ) {
        CMBVector column;
        column.ReadFromJsonObject(jsonobject[key].toObject());
        operator[](key.toStdString()) = column;
    }
    return true;
}
string CMBVectorSet::ToString()
{
    string s;
    for (map<string,CMBVector>::iterator it = begin(); it!=end();it++)
    {
        s+= "\n";
        s+=it->first + ":\n";
        s+=it->second.ToString()+="\n";
    }
    return s;
}

unsigned int CMBVectorSet::MaxSize() const
{
    int max_size=0;
    for (map<string,CMBVector>::const_iterator it = begin(); it!=end();it++)
    {
        if (it->second.num>max_size)
            max_size = it->second.num;
    }
    return max_size;
}

double CMBVectorSet::max() const
{
    double out = -1e23;
    for (map<string,CMBVector>::const_iterator it = cbegin(); it!=cend();it++)
    {
        out = std::max(out,it->second.max());
    }
    return out;
};
double CMBVectorSet::min() const
{
    double out = 1e23;
    for (map<string,CMBVector>::const_iterator it = cbegin(); it!=cend();it++)
    {
        out = std::min(out,it->second.min());
    }
    return out;
}


QTableWidget *CMBVectorSet::ToTable()
{
    QTableWidget *tablewidget = new QTableWidget();
    tablewidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablewidget->setColumnCount(size()*2);
    tablewidget->setRowCount(MaxSize());
    QStringList rowheaders;
    QStringList colheaders;
    int colno = 0;
    for (map<string,CMBVector>::iterator column=begin(); column!=end(); column++)
    {
        colheaders<<QString::fromStdString(column->first)<<QString::fromStdString(column->first)+"-Value";
        for (int i=0; i<column->second.getsize(); i++)
        {
            tablewidget->setItem(i,colno*2,new QTableWidgetItem(QString::fromStdString(column->second.Label(i))));
            tablewidget->setItem(i,colno*2+1,new QTableWidgetItem(QString::number(column->second[i])));
        }
        colno++;
    }

    tablewidget->setHorizontalHeaderLabels(colheaders);
    //tablewidget->setVerticalHeaderLabels(rowheaders);
    return tablewidget;
}
bool CMBVectorSet::writetofile(QFile* file)
{
    file->write(QString::fromStdString(ToString()).toUtf8());
    return true;
}

string CMBVectorSet::Label(string column,int j) const
{
    if (count(column)!=0)
        return (at(column).Label(j));
    else
        return "";
}
double CMBVectorSet::valueAt(const string &columnlabel, int j ) const
{
    if (count(columnlabel)!=0)
        return (at(columnlabel).valueAt(j));
    else
        return 0;
}
CMBVector &CMBVectorSet::GetColumn(const string columnlabel)
{
    return at(columnlabel);
}
void CMBVectorSet::Append(const string &columnlabel,const CMBVector &vector)
{
    this->operator[](columnlabel) = vector;
}
