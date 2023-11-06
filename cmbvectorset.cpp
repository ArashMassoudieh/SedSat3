#include "cmbvectorset.h"

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
        s+=it->first;
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

QTableWidget *CMBVectorSet::ToTable()
{
    QTableWidget *tablewidget = new QTableWidget();
    tablewidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablewidget->setColumnCount(size());
    tablewidget->setRowCount(MaxSize());
    QStringList rowheaders;
    QStringList colheaders;

    colheaders << "Value";

    for (int i=0; i<getsize(); i++)
    {
        rowheaders << QString::fromStdString(labels[i]);
        if (!boolean_values)
            tablewidget->setItem(i,0, new QTableWidgetItem(QString::number(valueAt(i))));
        else
        {
            if (valueAt(i)==1)
                tablewidget->setItem(i,0, new QTableWidgetItem("Fail"));
            else
                tablewidget->setItem(i,0, new QTableWidgetItem("Pass"));

        }
    }

    tablewidget->setHorizontalHeaderLabels(colheaders);
    tablewidget->setVerticalHeaderLabels(rowheaders);
    return tablewidget;
}
bool CMBVectorSet::writetofile(QFile* file)
{

}
double CMBVectorSet::valueAt(int i) const
{

}
string CMBVectorSet::Label(string column,int j) const
{

}
double CMBVectorSet::valueAt(const string &columnlabel, int j ) const
{

}
CMBVector &CMBVectorSet::GetColumn(const string columnlabel)
{

}
void CMBVectorSet::Append(const string &columnlabel,const CMBVector &vectorset)
{

}
