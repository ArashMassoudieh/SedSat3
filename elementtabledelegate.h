// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

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

    ElementTableDelegate(SourceSinkData *data, QObject *parent);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;

    /// Toggles the "Include in analysis" column with a single click
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &index) override;


public slots:

private:
    SourceSinkData *Data;
};

#endif // ELEMENTTABLEDELEGATE_H
