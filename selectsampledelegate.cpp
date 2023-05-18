#include "selectsampledelegate.h"

#include "QCheckBox"
#include "QComboBox"
#include "QLineEdit"
#include "QLabel"

SelectSampleDelegate::SelectSampleDelegate(SourceSinkData *_Data, QObject *parent)
{
    Data=_Data;
}

void SelectSampleDelegate::SetMode(mode _mode)
{
    Mode = _mode;
    if (Mode == mode::samples)
    {
        numberofcolumns = 3;
        columnTypes.resize(numberofcolumns);
        columnTypes[0] = column_type::name;
        columnTypes[1] = column_type::yesno;
        columnTypes[2] = column_type::string;
    }
    if (Mode == mode::regressions)
    {
        numberofcolumns = 8;
        columnTypes.resize(numberofcolumns);
        columnTypes[0] = column_type::name;
        columnTypes[1] = column_type::number;
        columnTypes[2] = column_type::number;
        columnTypes[3] = column_type::number;
        columnTypes[4] = column_type::yesno;
        columnTypes[5] = column_type::number;
        columnTypes[6] = column_type::number;
        columnTypes[7] = column_type::yesno;
    }
}

QWidget *SelectSampleDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                      const QModelIndex &index) const
{
    if (columnTypes[index.column()] == column_type::name) return QStyledItemDelegate::createEditor(parent, option, index);
    if (columnTypes[index.column()] == column_type::yesno)
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
    if (columnTypes[index.column()]==column_type::name)
    {
        QLabel *label = static_cast<QLabel*>(editor);
        label->setText(var.toString());
    }
    if (columnTypes[index.column()]==column_type::yesno)
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
    if (columnTypes[index.column()] == column_type::name) QStyledItemDelegate::setModelData(editor, model, index);
    if (columnTypes[index.column()] == column_type::yesno)
    {
        QCheckBox *chkbox = static_cast<QCheckBox*>(editor);

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

