// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#include "toolboxitem.h"

ToolBoxItem::ToolBoxItem():QStandardItem()
{

}


ToolBoxItem::ToolBoxItem(SourceSinkData *_data, const QString &itemtext):QStandardItem(itemtext)
{
    Data = _data;

}

QVariant ToolBoxItem::data(int role) const
{
    if (role == Qt::ForegroundRole)
    {
        if (Data->ToolsUsed(data(Qt::UserRole+1).toString().toStdString()))
            return QColor(Qt::blue);
        else
            return QStandardItem::data(role);
    }
    else
        return QStandardItem::data(role);
}
