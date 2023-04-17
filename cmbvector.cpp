#include "cmbvector.h"
#include "QJsonArray"
#include "Vector.h"
#include <QFile>

CMBVector::CMBVector():CVector(),Interface()
{

}

CMBVector::CMBVector(const CMBVector& mp):CVector(mp)
{
    labels = mp.labels;
    boolean_values = mp.boolean_values;

}

CMBVector::CMBVector(const CVector& mp):CVector(mp)
{
    labels.resize(getsize());
}

CMBVector::CMBVector(int n):CVector(n), Interface()
{
    labels.resize(n);

}

CMBVector& CMBVector::operator=(const CMBVector &mp)
{
    labels = mp.labels;
    boolean_values = mp.boolean_values;
    CVector::operator=(mp);
    return *this;

}

CMBVector& CMBVector::operator=(const CVector &mp)
{
    CVector::operator=(mp);
    labels.resize(getsize());
    return *this; 
}

QJsonObject CMBVector::toJsonObject()
{
    QJsonObject out;
    QJsonArray vector;
    QJsonArray elementlabels;
    for (int i=0; i<getsize(); i++)
    {
        vector.append(valueAt(i));
        elementlabels.append(QString::fromStdString(labels[i]));
    }
    out["Vector"] = vector;
    out["Labels"] = elementlabels;
    return out;
}
bool CMBVector::ReadFromJsonObject(const QJsonObject &jsonobject)
{
    QJsonArray array = jsonobject["Vector"].toArray();
    QJsonArray elementlabels = jsonobject["Labels"].toArray();
    vec.resize(array.size());
    num = array.size();
    labels.resize(array.size());
    for (unsigned int i=0; i<array.size(); i++)
    {
        vec[i]=array[i].toDouble();
        labels[i]=elementlabels[i].toString().toStdString();
    }

    return true;
}


string CMBVector::ToString()
{
    string out;
    if (!boolean_values)
    {   for (int j=0; j<getsize(); j++)
        {
            out += labels[j] + "," + QString::number(valueAt(j)).toStdString()+"\n";
        }
    }
    else
    {   for (int j=0; j<getsize(); j++)
        {
            if (valueAt(j)==1)
                out += labels[j] + ", Did't pass \n";
            else
                out += labels[j] + ", Pass \n";
        }
    }

    return out;
}

bool CMBVector::writetofile(QFile* file)
{
    file->write(QString::fromStdString(ToString()).toUtf8());
    return true;
}

double CMBVector::valueAt(int i)
{
    return CVector::operator[](i);
}

QTableWidget *CMBVector::ToTable()
{
    QTableWidget *tablewidget = new QTableWidget();
    tablewidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablewidget->setColumnCount(1);
    tablewidget->setRowCount(getsize());
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
                tablewidget->setItem(i,0, new QTableWidgetItem("Did not pass"));
            else
                tablewidget->setItem(i,0, new QTableWidgetItem("Pass"));

        }
    }

    tablewidget->setHorizontalHeaderLabels(colheaders);
    tablewidget->setVerticalHeaderLabels(rowheaders);
    return tablewidget;
}

CVector CMBVector::toVector() const
{
    CVector out(num);
    out.vec = vec;
    return out;
}
