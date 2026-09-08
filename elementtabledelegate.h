#ifndef ELEMENTTABLEDELEGATE_H
#define ELEMENTTABLEDELEGATE_H


#include <QAbstractTableModel>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>
#include "sourcesinkdata.h"

class ElementTableDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:

    ElementTableDelegate(SourceSinkData *_Data, QObject *parent);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;


public slots:

private:
    SourceSinkData *Data;
};

#endif // ELEMENTTABLEDELEGATE_H
