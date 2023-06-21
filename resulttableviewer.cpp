#include "resulttableviewer.h"
#include "ui_resulttableviewer.h"
#include <QFile>
#include <QFileDialog>

ResultTableViewer::ResultTableViewer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ResultTableViewer)
{

    Qt::WindowFlags flags = Qt::Window | Qt::WindowSystemMenuHint
                                | Qt::WindowMinMaxButtonsHint
                                | Qt::WindowCloseButtonHint;
    this->setWindowFlags(flags);
    ui->setupUi(this);
    connect(ui->ExporttoCSV, SIGNAL(clicked()),this,SLOT(on_ExportToCSV()));
}

ResultTableViewer::~ResultTableViewer()
{
    delete ui;
}

void ResultTableViewer::SetTable(QTableWidget *_tablewidget)
{
    tablewidget = _tablewidget;
    ui->verticalLayout->addWidget(tablewidget);
}

void ResultTableViewer::on_ExportToCSV()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save"), "",
        tr("csv file (*.csv)"));

    if (!fileName.contains("."))
        fileName+=".csv";
    QFile file(fileName);

    file.open(QIODevice::WriteOnly | QIODevice::Text);
    if (!file.isOpen())
        return;

    QString string;
    for (int column=0; column<tablewidget->columnCount(); column++)
        string+=","+tablewidget->horizontalHeaderItem(column)->text();
    string += "\n";
    for (int row=0; row<tablewidget->rowCount(); row++)
    {
        string+=tablewidget->verticalHeaderItem(row)->text();
        for (int column=0; column<tablewidget->columnCount(); column++)
        {
            string+=","+tablewidget->item(row,column)->text();
        }
        string += "\n";
    }

    file.write(string.toUtf8());
    file.close();
}
