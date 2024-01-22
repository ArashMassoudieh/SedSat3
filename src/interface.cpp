#include "interface.h"

Interface::Interface()
{

}

Interface::Interface(const Interface &intf)
{
    Options = intf.Options;
}
Interface& Interface::operator=(const Interface &intf)
{
    Options = intf.Options;
    return *this;
}

string Interface::ToString()
{
    return "";
}

QJsonObject Interface::toJsonObject()
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
