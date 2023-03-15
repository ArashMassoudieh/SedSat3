#include "resulttableviewer.h"
#include "ui_resulttableviewer.h"

ResultTableViewer::ResultTableViewer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ResultTableViewer)
{
    ui->setupUi(this);
}

ResultTableViewer::~ResultTableViewer()
{
    delete ui;
}

void ResultTableViewer::SetTable(QTableWidget *tablewidget)
{
    ui->horizontalLayout->addWidget(tablewidget);
}
