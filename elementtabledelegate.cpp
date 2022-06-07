#include "elementtabledelegate.h"
#include "QCheckBox"
#include "QComboBox"
#include "QLineEdit"
#include "QLabel"

ElementTableDelegate::ElementTableDelegate(QObject *parent)
{

}

QWidget *ElementTableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                      const QModelIndex &index) const
{
    if (index.column() == 0) return QStyledItemDelegate::createEditor(parent, option, index);


    if (index.column()==1)
    {
        QCheckBox *editor = new QCheckBox(parent);
        QVariant var = index.data(Qt::DisplayRole);
        if (var.toString()=="Yes")
            editor->setCheckState(Qt::CheckState::Checked);
        else
            editor->setCheckState(Qt::CheckState::Unchecked);
        return editor;
    }
    if (index.column()==2)
    {
        QComboBox *editor = new QComboBox(parent);
        editor->setCurrentText(index.data().toString());
        return editor;
    }
    if (index.column()==3)
    {
        QLineEdit *editor = new QLineEdit(parent);
        QString text = index.data(Qt::DisplayRole).toString();
        editor->setText(text);
        return editor;
    }
    return nullptr;

}

void ElementTableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QVariant var = index.data(Qt::DisplayRole);
    if (index.column()==0)
    {
        QLabel *label = static_cast<QLabel*>(editor);
        label->setText(var.toString());
    }
    if (index.column()==1)
    {
        QCheckBox *checkbox = static_cast<QCheckBox*>(editor);
        if (var.toString()=="Yes")
            checkbox->setCheckState(Qt::CheckState::Checked);
        else
            checkbox->setCheckState(Qt::CheckState::Unchecked);
        checkbox->show();
        return;
    }
    if (index.column()==2)
    {
        QComboBox *combo = static_cast<QComboBox*>(editor);
        combo->setCurrentText(var.toString());
    }
    if (index.column()==3)
    {
        QLineEdit *lineedit = static_cast<QLineEdit*>(editor);
        lineedit->setText(var.toString());
    }
}
void ElementTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                  const QModelIndex &index) const
{

}

void ElementTableDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

void ElementTableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
     QStyledItemDelegate::paint(painter, option, index);
}
