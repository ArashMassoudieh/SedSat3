#ifndef RESULTSETITEM_H
#define RESULTSETITEM_H

#include <QStandardItem>
#include "results.h"

class ResultSetItem : public QStandardItem
{
public:
    ResultSetItem();
    ResultSetItem(const QString &str);
    Results *result=nullptr;
};

#endif // RESULTSETITEM_H
