#include "dialogchooseexcelsheets.h"
#include "ui_dialogchooseexcelsheets.h"
#include <QListWidgetItem>

DialogChooseExcelSheets::DialogChooseExcelSheets(QWidget *parent, QStringList **_items) :
    QDialog(parent),
    ui(new Ui::DialogChooseExcelSheets)
{
    *_items = new QStringList();
    items = *_items;
    ui->setupUi(this);
}

DialogChooseExcelSheets::~DialogChooseExcelSheets()
{
    delete ui;
}

void DialogChooseExcelSheets::AddItem(const QString &item)
{
    QListWidgetItem *listItem = new QListWidgetItem(item,ui->listWidget);
    listItem->setCheckState(Qt::Unchecked);
    ui->listWidget->addItem(listItem);

}

void DialogChooseExcelSheets::reject()
{
    QDialog::reject();
}
void DialogChooseExcelSheets::accept()
{

    for(int i = 0; i < ui->listWidget->count(); ++i)
    {
        QListWidgetItem* item = ui->listWidget->item(i);
        if (item->checkState())
        {
            items->append(item->text());
        }
    }
    QDialog::accept();
}
