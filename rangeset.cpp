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
