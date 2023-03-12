#include "rangeset.h"

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
QJsonObject RangeSet::toJsonObject()
{
    QJsonObject out;
    for (map<string,Range>::iterator it=begin(); it!=end(); it++)
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
        this->at(key.toStdString()) = range;
    }
    return true;
}
string RangeSet::ToString()
{
    string out;
    for (map<string,Range>::iterator it=begin(); it!=end(); it++)
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
