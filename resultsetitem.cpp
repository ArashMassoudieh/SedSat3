#include "resultsetitem.h"

ResultSetItem::ResultSetItem():QStandardItem()
{

}

ResultSetItem::ResultSetItem(const QString &str):QStandardItem(str)
{
    this->setToolTip(str);
}
