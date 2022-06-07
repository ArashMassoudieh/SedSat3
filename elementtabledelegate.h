#ifndef ELEMENTTABLEDELEGATE_H
#define ELEMENTTABLEDELEGATE_H


#include <QAbstractTableModel>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>


class ElementTableDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:

    ElementTableDelegate(QObject *parent);

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

};

#endif // ELEMENTTABLEDELEGATE_H
