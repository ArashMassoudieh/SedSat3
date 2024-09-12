#include "cmbvector.h"
#include "QJsonArray"
#include "Vector.h"
#include <QFile>

CMBVector::CMBVector():CVector(),Interface()
{

}

CMBVector::CMBVector(const CMBVector& mp):CVector(mp), Interface()
{
    labels = mp.labels;
    boolean_values = mp.boolean_values;
    highlimit = mp.highlimit;
    lowlimit = mp.lowlimit;
    highlightoutsideoflimit = mp.highlightoutsideoflimit;

}

CMBVector::CMBVector(const CVector& mp):CVector(mp), Interface()
{
    labels.resize(getsize());
}

CMBVector::CMBVector(const CVector_arma& mp): Interface()
{
    labels.resize(getsize());
    CVector::operator=(mp);
}

CMBVector::CMBVector(int n):CVector(n), Interface()
{
    labels.resize(n);

}

CMBVector& CMBVector::operator=(const CMBVector &mp)
{
    labels = mp.labels;
    boolean_values = mp.boolean_values;
    highlimit = mp.highlimit;
    lowlimit = mp.lowlimit;
    highlightoutsideoflimit = mp.highlightoutsideoflimit;
    CVector::operator=(mp);
    return *this;

}

CMBVector& CMBVector::operator=(const CVector_arma &mp)
{
    CVector::operator=(mp);
    labels.resize(mp.size());
    return *this;
}

CMBVector& CMBVector::operator=(const CVector &mp)
{
    CVector::operator=(mp);
    labels.resize(getsize());
    return *this; 
}

CMBVector& CMBVector::operator=(const CVector_arma &mp)
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
    out["XAxisLabel"] = XAxisLabel();
    out["YAxisLabel"] = YAxisLabel();
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

    if (jsonobject.contains("XAxisLabel"))
    {
        SetXAxisLabel(jsonobject["XAxisLabel"].toString());
    }

    if (jsonobject.contains("YAxisLabel"))
    {
        SetXAxisLabel(jsonobject["YAxisLabel"].toString());
    }
    return true;
}


string CMBVector::ToString()
{
    string out;
    out += XAxisLabel().toStdString() + "," + YAxisLabel().toStdString() + "\n";
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
                out += labels[j] + ", Fail \n";
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

double CMBVector::valueAt(int i) const
{
    return CVector::operator[](i);
}

double CMBVector::valueAt(const string &label ) const
{
    for (int i=0; i<getsize(); i++)
        if (labels[i]==label)
            return valueAt(i);
    return -999;
}

int CMBVector::LookupLabel(const string &label ) const
{
    for (int i=0; i<getsize(); i++)
        if (labels[i]==label)
            return i;
    return -1;
}

QTableWidget *CMBVector::ToTable()
{
    QTableWidget *tablewidget = new QTableWidget();
    tablewidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tablewidget->setColumnCount(1);
    tablewidget->setRowCount(getsize());
    QStringList rowheaders;
    QStringList colheaders;

    colheaders << YAxisLabel();

    for (int i=0; i<getsize(); i++)
    {
        rowheaders << QString::fromStdString(labels[i]);
        if (!boolean_values)
        {   tablewidget->setItem(i,0, new QTableWidgetItem(QString::number(valueAt(i))));
            if (highlightoutsideoflimit)
            {
                if (valueAt(i)>highlimit || valueAt(i)<lowlimit)
                {
                    tablewidget->item(i,0)->setForeground(QColor(Qt::red));
                }
            }
        }
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

CVector CMBVector::toVector() const
{
    CVector out(num);
    out.vec = vec;
    return out;
}

CMBVector CMBVector::Sort(const CMBVector &sortvector) const
{
    CMBVector eliminated;
    if (sortvector.getsize()==0)
        eliminated = *this;
    else
        eliminated = sortvector;
    CMBVector out;
    for (int i=0; i<getsize(); i++)
    {
        string max_element = eliminated.MaxElement();
        out.append(max_element,valueAt(max_element));
        eliminated = eliminated.Eliminate(max_element);
    }
    return out;
}
CMBVector CMBVector::AbsSort(const CMBVector &sortvector) const
{
    CMBVector eliminated;
    if (sortvector.getsize()==0)
        eliminated = *this;
    else
        eliminated = sortvector;
    CMBVector out;
    for (int i=0; i<getsize(); i++)
    {
        string max_element = eliminated.MaxAbsElement();
        out.append(max_element,valueAt(max_element));
        eliminated = eliminated.Eliminate(max_element);
    }
    return out;
}
string CMBVector::MaxElement() const
{
    double val = -1e24;
    string out;
    for (int i=0; i<getsize(); i++)
    {
        if (val<valueAt(i))
        {   val = valueAt(i);
            out = Label(i);
        }
    }
    return out;
}
string CMBVector::MaxAbsElement() const
{
    double val = -1e24;
    string out;
    for (int i=0; i<getsize(); i++)
    {
        if (val<fabs(valueAt(i)))
        {   val = fabs(valueAt(i));
            out = Label(i);
        }
    }
    return out;
}
CMBVector CMBVector::Eliminate(const string &element) const
{
    CMBVector out;
    for (int i=0; i<getsize(); i++)
    {
        if (Label(i)!=element)
            out.append(Label(i),valueAt(i));
    }
    return out;
}

void CMBVector::append(const string &label, const double &val)
{
    vec.push_back(val);
    labels.push_back(label);
    num = vec.size();

}

CMBVector CMBVector::ExtractWithinRange(const double &lowval, const double &highval) const
{
    CMBVector out;
    for (int i=0; i<size(); i++)
    {
        if (valueAt(i)<highval && valueAt(i)>lowval)
            out.append(labels[i],valueAt(i));
    }
    return out;
}

CMBVector CMBVector::Extract(int start, int end) const
{
    CMBVector out;
    for (int i=start; i<=end; i++)
    {
        out.append(Label(i),valueAt(i));
    }
    return out;
}


CMBVector operator+(const CMBVector& V1, const CMBVector& V2)
{
    CMBVector out = V1;
    out.SetLabels(V1.Labels());
    out += V2;
    return out;
}
CMBVector operator+(double d, const CMBVector& V1)
{
    CMBVector out = V1;
    out.SetLabels(V1.Labels());
    out += d;
    return out;
}
CMBVector operator+(const CMBVector& V1, double d)
{
    CMBVector out = V1;
    out.SetLabels(V1.Labels());
    out += d;
    return out;
}
CMBVector operator-(const CMBVector& V1, const CMBVector& V2)
{
    CMBVector out = V1;
    out.SetLabels(V1.Labels());
    out -= V2;
    return out;
}
CMBVector operator-(double d, const CMBVector& V1)
{
    CMBVector out(V1.size());
    out.SetLabels(V1.Labels());
    out -= V1;
    out += d;
    return out;
}
CMBVector operator-(const CMBVector& V1, double d)
{
    CMBVector out = V1;
    out.SetLabels(V1.Labels());
    out -= d;
    return out;
}
CMBVector operator*(const CMBVector& V1, const CMBVector& V2)
{
    CMBVector out = V1;
    out.SetLabels(V1.Labels());
    out *= V2;
    return out;
}
CMBVector operator*(double d, const CMBVector& V1)
{
    CMBVector out = V1;
    out.SetLabels(V1.Labels());
    out *= d;
    return out;
}
CMBVector operator/(const CMBVector& V1, double d)
{
    CMBVector out = V1;
    out.SetLabels(V1.Labels());
    out /= d;
    return out;
}
