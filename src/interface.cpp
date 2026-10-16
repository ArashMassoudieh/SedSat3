// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#include "interface.h"

Interface::Interface()
{

}

Interface::Interface(const Interface &intf)
{
    Options = intf.Options;
    XAxis_Label = intf.XAxis_Label;
    YAxis_Label = intf.YAxis_Label;
}
Interface& Interface::operator=(const Interface &intf)
{
    Options = intf.Options;
    XAxis_Label = intf.XAxis_Label;
    YAxis_Label = intf.YAxis_Label;
    return *this;
}

string Interface::ToString() const
{
    return "";
}

QJsonObject Interface::toJsonObject() const
{
    return QJsonObject();
}

bool Interface::ReadFromJsonObject(const QJsonObject &jsonobject)
{
    return true;
}


bool Interface::writetofile(QFile* file)
{
    return false;
}

bool Interface::Read(const QStringList &strlist)
{
    return false;
}

QTableWidget *Interface::ToTable()
{
    QTableWidget *tablewidget = new QTableWidget();
    return tablewidget;
}
