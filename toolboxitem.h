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
