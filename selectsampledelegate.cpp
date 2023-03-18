#include "selectsampledelegate.h"

#include "QCheckBox"
#include "QComboBox"
#include "QLineEdit"
#include "QLabel"

SelectSampleDelegate::SelectSampleDelegate(SourceSinkData *_Data, QObject *parent)
{
    Data=_Data;
}

QWidget *SelectSampleDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                      const QModelIndex &index) const
{
    if (index.column() == 0) return QStyledItemDelegate::createEditor(parent, option, index);
    if (index.column()==1)
    {
        QCheckBox *editor = new QCheckBox(parent);
        editor->setChecked(index.data(Qt::DisplayRole).toBool());
        return editor;
    }

    return nullptr;

}

void SelectSampleDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QVariant var = index.data(Qt::DisplayRole);
    if (index.column()==0)
    {
        QLabel *label = static_cast<QLabel*>(editor);
        label->setText(var.toString());
    }
    if (index.column()==1)
    {
        QCheckBox *chkbox = static_cast<QCheckBox*>(editor);
        if (var.toString()=="No")
            chkbox->setChecked(false);
        else
            chkbox->setChecked(true);
    }
}
void SelectSampleDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                  const QModelIndex &index) const
{
    if (index.column() == 0) QStyledItemDelegate::setModelData(editor, model, index);
    QString element = model->data(index.sibling(index.row(), 0)).toString();

    if (index.column() == 1)
    {
        QCheckBox *chkbox = static_cast<QCheckBox*>(editor);
        qDebug()<<chkbox->checkState();
        if (chkbox->checkState()==Qt::CheckState::Checked)
            model->setData(index,true);
        else
            model->setData(index,false);
        return;
    }
}

void SelectSampleDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

void SelectSampleDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
     QStyledItemDelegate::paint(painter, option, index);
}

