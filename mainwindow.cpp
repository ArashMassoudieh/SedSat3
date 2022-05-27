#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QDebug"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionImport_Elemental_Profile_from_Excel,SIGNAL(triggered()),this,SLOT(on_import_excel()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_import_excel()
{
    qDebug()<<"here!";
}

