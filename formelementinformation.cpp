// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#include "formelementinformation.h"
#include "ui_formelementinformation.h"
#include "elementstablemodel.h"
#include <QSignalBlocker>

FormElementInformation::FormElementInformation(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormElementInformation)
{
    ui->setupUi(this);
    // The label is fixed and the check state carries the meaning: checked
    // means every element is included. (The label used to flip between
    // "Include all" and "Exclude all", so a checked box read "Include all
    // elements" while every element had just been excluded.)
    ui->Include_Exclude_All_checkBox->setText(tr("Include all elements"));
    connect(ui->Include_Exclude_All_checkBox,SIGNAL(stateChanged(int)),this,SLOT(on_Include_Exclude_Change()));

}

FormElementInformation::~FormElementInformation()
{
    delete ui;
}

QTableView *FormElementInformation::table()
{
    return ui->tableView;
}

void FormElementInformation::setModel(ElementTableModel *model)
{
    ui->tableView->setModel(model);
    connect(model, &ElementTableModel::editCompleted, this, &FormElementInformation::SyncIncludeAllCheckBox);
    SyncIncludeAllCheckBox();
}

void FormElementInformation::SyncIncludeAllCheckBox()
{
    ElementTableModel* model = static_cast<ElementTableModel*>(table()->model());
    if (model == nullptr || model->Data == nullptr)
        return;

    bool all_included = true;
    for (const auto& element : *model->Data->GetElementInformation())
    {
        if (!element.second.include_in_analysis)
        {
            all_included = false;
            break;
        }
    }

    // Reflect the data without acting on it
    QSignalBlocker blocker(ui->Include_Exclude_All_checkBox);
    ui->Include_Exclude_All_checkBox->setChecked(all_included);
}

void FormElementInformation::on_Include_Exclude_Change()
{
    ElementTableModel* model = static_cast<ElementTableModel*>(table()->model());
    if (model == nullptr || model->Data == nullptr)
        return;

    const bool include_all = (ui->Include_Exclude_All_checkBox->checkState() == Qt::CheckState::Checked);
    model->Data->IncludeExcludeAllElements(include_all);
    table()->viewport()->update();
}
