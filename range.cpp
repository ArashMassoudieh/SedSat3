#include "range.h"
#include <QJsonArray>
#include <QJsonObject>

Range::Range():Interface()
{
    range.resize(2);
}

Range::Range(const Range &rhs):Interface(rhs)
{
    range = rhs.range;
}
Range& Range::operator = (const Range &rhs)
{
    range = rhs.range;
    Interface::operator=(rhs);
    return *this;
}

QJsonObject Range::toJsonObject()
{
    QJsonObject out;
    QJsonArray array;
    array[0] = range[0];
    array[1] = range[1];
    out["range"]=array;
    return out;
}
bool Range::ReadFromJsonObject(const QJsonObject &jsonobject)
{
    range.resize(2);
    range[0]=jsonobject["range"].toArray()[0].toDouble();
    range[1]=jsonobject["range"].toArray()[1].toDouble();
    return true;
}
string Range::ToString()
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
double Range::Get(_range lowhigh)
{
    if (lowhigh==_range::high)
        return range[1];
    else
        return range[0];
}

