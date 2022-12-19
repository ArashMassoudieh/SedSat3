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

string Contribution::ToString()
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

QJsonObject Contribution::toJsonObject()
{
    QJsonObject out;
    for (map<string,double>::const_iterator it=cbegin(); it!=cend(); it++ )
    {
        out[QString::fromStdString(it->first)] = it->second;
    }
    return out;
}
