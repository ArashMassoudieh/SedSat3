#include "elementtabledelegate.h"
#include "QCheckBox"
#include "QComboBox"
#include "QLineEdit"
#include "QLabel"

ElementTableDelegate::ElementTableDelegate(SourceSinkData *_Data, QObject *parent)
{
    Data=_Data;
}

QWidget *ElementTableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                      const QModelIndex &index) const
{
    if (index.column() == 0) return QStyledItemDelegate::createEditor(parent, option, index);


    if (index.column()==1)
    {
        QComboBox *editor = new QComboBox(parent);
        editor->addItem("Element"); editor->addItem("Isotope"); editor->addItem("Particle Size");editor->addItem("Organic Carbon");editor->addItem("Exclude");
        string element_name = index.sibling(index.row(),0).data(Qt::DisplayRole).toString().toStdString();
        if (Data->GetElementInformation(element_name)->Role == element_information::role::do_not_include)
            editor->setCurrentText("Exclude");
        if (Data->GetElementInformation(element_name)->Role == element_information::role::element)
            editor->setCurrentText("Element");
        if (Data->GetElementInformation(element_name)->Role == element_information::role::isotope)
            editor->setCurrentText("Isotope");
        if (Data->GetElementInformation(element_name)->Role == element_information::role::particle_size)
            editor->setCurrentText("Particle Size");
        if (Data->GetElementInformation(element_name)->Role == element_information::role::organic_carbon)
            editor->setCurrentText("Organic Carbon");


        return editor;
    }
    if (index.column()==2)
    {
        QComboBox *editor = new QComboBox(parent);
        vector<string> element_names = Data->GetElementNames();
        if (index.sibling(index.row(),1).data(Qt::DisplayRole)!="Isotope")
        {   editor->setEnabled(false);
            editor->setCurrentText("");
        }
        else
            editor->setEnabled(true);


        for (unsigned int i=0; i<element_names.size(); i++)
            if (element_names[i]!=index.sibling(index.row(),0).data(Qt::DisplayRole).toString().toStdString())
                editor->addItem(QString::fromStdString(element_names[i]));
        editor->setCurrentText(index.data().toString());
        return editor;
    }
    if (index.column()==3)
    {
        QLineEdit *editor = new QLineEdit(parent);
        if (index.sibling(index.row(),1).data(Qt::DisplayRole)!="Isotope")
        {   editor->setEnabled(false);
            editor->setText("");
        }
        else
            editor->setEnabled(true);

        QString text = index.data(Qt::DisplayRole).toString();

        editor->setText(text);
        return editor;
    }
    if (index.column()==4)
    {
        QCheckBox *editor = new QCheckBox(parent);
        editor->setChecked(index.data(Qt::DisplayRole).toBool());
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
        QComboBox *combo = static_cast<QComboBox*>(editor);
        string element_name = index.sibling(index.row(),0).data(Qt::DisplayRole).toString().toStdString();
        if (Data->GetElementInformation(element_name)->Role == element_information::role::do_not_include)
            combo->setCurrentText("Exclude");
        if (Data->GetElementInformation(element_name)->Role == element_information::role::element)
            combo->setCurrentText("Element");
        if (Data->GetElementInformation(element_name)->Role == element_information::role::isotope)
            combo->setCurrentText("Isotope");
        if (Data->GetElementInformation(element_name)->Role == element_information::role::particle_size)
            combo->setCurrentText("Particle Size");
        if (Data->GetElementInformation(element_name)->Role == element_information::role::organic_carbon)
            combo->setCurrentText("Organic Carbon");

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
    if (index.column()==4)
    {
        QCheckBox *chkbox = static_cast<QCheckBox*>(editor);
        if (var.toString()=="No")
            chkbox->setChecked(false);
        else
            chkbox->setChecked(true);
    }
}
void ElementTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                  const QModelIndex &index) const
{
    if (index.column() == 0) QStyledItemDelegate::setModelData(editor, model, index);
    QString element = model->data(index.sibling(index.row(), 0)).toString();

    if (index.column() == 1)
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        model->setData(index,comboBox->currentText());
        return;
    }
    if (index.column() == 2)
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        model->setData(index,comboBox->currentText());
        return;
    }
    if (index.column() == 3)
    {
        QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
        model->setData(index,lineEdit->text());
        return;
    }
    if (index.column() == 4)
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
