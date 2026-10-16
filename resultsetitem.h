// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

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

private:

};

#endif // RESULTSETITEM_H
