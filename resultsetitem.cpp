// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#include "resultsetitem.h"

ResultSetItem::ResultSetItem():QStandardItem()
{

}

ResultSetItem::ResultSetItem(const QString &str):QStandardItem(str)
{
    this->setToolTip(str);
}
