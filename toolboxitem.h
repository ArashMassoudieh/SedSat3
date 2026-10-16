// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#ifndef TOOLBOXITEM_H
#define TOOLBOXITEM_H

#include <QStandardItem>
#include "sourcesinkdata.h"

class ToolBoxItem : public QStandardItem
{
public:
    ToolBoxItem();
    ToolBoxItem(SourceSinkData *_data, const QString &itemtext);
    QVariant data(int role = Qt::UserRole + 1) const override;

private:
    SourceSinkData *Data;
};

#endif // TOOLBOXITEM_H
