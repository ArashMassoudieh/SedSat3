// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#include "formelementinformation.h"
#include "ui_formelementinformation.h"
#include "elementstablemodel.h"

FormElementInformation::FormElementInformation(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormElementInformation)
{
    ui->setupUi(this);
    ui->Include_Exclude_All_checkBox->setText("Exclude all elements");
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


void FormElementInformation::on_Include_Exclude_Change()
{
    if (ui->Include_Exclude_All_checkBox->checkState()==Qt::CheckState::Checked)
    {
        ui->Include_Exclude_All_checkBox->setText("Include all elements");
        static_cast<ElementTableModel*>(table()->model())->Data->IncludeExcludeAllElements(false);
    }
    else
    {
        ui->Include_Exclude_All_checkBox->setText("Exclude all elements");
        static_cast<ElementTableModel*>(table()->model())->Data->IncludeExcludeAllElements(true);
    }
}

