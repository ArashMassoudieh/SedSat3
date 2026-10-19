// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#include "interface.h"

Interface::Interface()
{

}

// Copies carry every member. (These used to copy only the options and axis
// labels, so a copied profile lost its notes, such as the outlier and
// bracketing flags, and its highlight limits.)
Interface::Interface(const Interface &intf) = default;
Interface& Interface::operator=(const Interface &intf) = default;

string Interface::ToString() const
{
    return "";
}

QJsonObject Interface::toJsonObject() const
{
    return QJsonObject();
}

bool Interface::ReadFromJsonObject(const QJsonObject &/*jsonobject*/)
{
    return true;
}


bool Interface::writetofile(QFile* /*file*/)
{
    return false;
}

bool Interface::Read(const QStringList &/*strlist*/)
{
    return false;
}

QTableWidget *Interface::ToTable()
{
    QTableWidget *tablewidget = new QTableWidget();
    return tablewidget;
}
