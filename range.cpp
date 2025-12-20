#include "range.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>

Range::Range():Interface()
{
    range.resize(2);
}

Range::Range(const Range &rhs):Interface(rhs)
{
    range = rhs.range;
    mean = rhs.mean;
    median = rhs.median;
    observed_value = rhs.observed_value;
}
Range& Range::operator = (const Range &rhs)
{
    observed_value = rhs.observed_value;
    range = rhs.range;
    mean = rhs.mean;
    median = rhs.median;
    Interface::operator=(rhs);
    return *this;
}

QJsonObject Range::toJsonObject() const
{
    QJsonObject out;
    QJsonArray array;
    array.append(range[0]);
    array.append(range[1]);
    out["range"]=array;
    out["value"]=observed_value;
    out["mean"]=mean;
    out["median"]=median;
    return out;
}
bool Range::ReadFromJsonObject(const QJsonObject &jsonobject)
{
    range.resize(2);
    range[0]=jsonobject["range"].toArray()[0].toDouble();
    range[1]=jsonobject["range"].toArray()[1].toDouble();
    observed_value = jsonobject["value"].toDouble();
    mean = jsonobject["mean"].toDouble();
    median = jsonobject["median"].toDouble();
    return true;
}
string Range::ToString() const
{
    return ("[" + QString::number(range[0])+ "," + QString::number(range[1]) + "]\n").toStdString();
}
bool Range::writetofile(QFile* file)
{
    file->write(QString::fromStdString(ToString()).toUtf8());
    return true;
}
bool Range::Read(const QStringList &strlist)
{
    return true;
}

void Range::Set(_range lowhigh,const double &value)
{
    if (lowhigh==_range::high)
        range[1]=value;
    else
        range[0]=value;
}
double Range::Get(_range lowhigh) const
{
    if (lowhigh==_range::high)
        return range[1];
    else
        return range[0];
}

