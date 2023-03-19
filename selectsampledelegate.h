#ifndef SELECTSAMPLEDELEGATE_H
#define SELECTSAMPLEDELEGATE_H

#include <QAbstractTableModel>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>
#include "sourcesinkdata.h"
#include "selectsampletablemodel.h"

enum class mode {samples, regressions};
enum class column_type {number, yesno, name};


class SelectSampleDelegate: public QStyledItemDelegate
{
public:

    SelectSampleDelegate(SourceSinkData *_Data, QObject *parent);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;
    void SetMode(mode _mode);

public slots:

private:
    SourceSinkData *Data;
    mode Mode;
    QVector<column_type> columnTypes;
    int numberofcolumns;


};

#endif // SELECTSAMPLEDELEGATE_H
