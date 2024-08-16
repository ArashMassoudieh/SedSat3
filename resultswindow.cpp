#include "resultswindow.h"
#include "ui_resultswindow.h"
#include "QTextBrowser"
#include "QDebug"
#include "QPushButton"
#include "generalchart.h"
#include "resultitem.h"
#include "resulttableviewer.h"

#include <QFileDialog>


ResultsWindow::ResultsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ResultsWindow)
{
    ui->setupUi(this);

}

ResultsWindow::~ResultsWindow()
{
    delete ui;
}

//void ResultsWindow::AppendText(const string &text)
//{
//    ui->textBrowser->append(QString::fromStdString(text));
//}

void ResultsWindow::AppendResult(const ResultItem &resultitem)
{

    result_item_counter++;
    QTextBrowser *textBrowser = new QTextBrowser(ui->scrollAreaWidgetContents);
    textBrowser->setObjectName(QString::number(result_item_counter)+":"+QString::fromStdString(resultitem.Name()));
    //textBrowser->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
    textBrowser->setTextColor(Qt::red);
    textBrowser->append(QString::fromStdString(resultitem.Name())+":");
    textBrowser->setTextColor(Qt::black);
    textBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QString str;
    if (resultitem.ShowAsString())
        str = QString::fromStdString(resultitem.Result()->ToString());
    else
    {
        if (resultitem.ShowGraph() && resultitem.ShowTable())
            str = QString::fromStdString("Push the graph or table button");
        else if (resultitem.ShowGraph())
            str = QString::fromStdString("Push the graph button");
        else if (resultitem.ShowTable())
            str = QString::fromStdString("Push the table button");

    }

    textBrowser->append(str);
    int count = 0;
    for(int i = 0;i < str.length();i++)
        if(str.at(i).cell() == '\n')
            count++;
    textBrowser->setMaximumHeight(textBrowser->fontMetrics().height() * (count+3));
    textBrowser->setMinimumHeight(textBrowser->fontMetrics().height() * (count+2));
    ui->gridLayout->setAlignment(Qt::AlignTop);
    ui->gridLayout->addWidget(textBrowser,ui->gridLayout->rowCount(),0);
    if (resultitem.ShowGraph())
    {
        QPushButton *pushButtonGraph = new QPushButton(this);
        QIcon iconGraph = QIcon(qApp->applicationDirPath()+"/../../resources/Icons/Graph.png");
        pushButtonGraph->setIcon(iconGraph);
        //pushButton->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum);
        pushButtonGraph->setMaximumWidth(20);

        if (QString::fromStdString(resultitem.Name()).contains(":"))
            pushButtonGraph->setObjectName(QString::fromStdString(resultitem.Name()));
        else
            pushButtonGraph->setObjectName(QString::number(result_item_counter) + ":" + QString::fromStdString(resultitem.Name()));

        ui->gridLayout->addWidget(pushButtonGraph,ui->gridLayout->rowCount()-1,1,1,1,Qt::AlignTop);
        connect(pushButtonGraph,SIGNAL(clicked()),this,SLOT(on_result_graph_clicked()));
    }

    QPushButton *pushButtonExport = new QPushButton(this);
    QIcon iconExport = QIcon(qApp->applicationDirPath()+"/../../resources/Icons/export.png");
    pushButtonExport->setIcon(iconExport);
    //pushButton->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum);
    pushButtonExport->setMaximumWidth(20);
    pushButtonExport->setObjectName(QString::number(result_item_counter)+":"+QString::fromStdString(resultitem.Name()));
    ui->gridLayout->addWidget(pushButtonExport,ui->gridLayout->rowCount()-1,2,1,1,Qt::AlignTop);
    connect(pushButtonExport,SIGNAL(clicked()),this,SLOT(on_result_export_clicked()));


    if (resultitem.ShowTable())
    {   QPushButton *pushButtonTable = new QPushButton(this);
        QIcon iconTable = QIcon(qApp->applicationDirPath()+"/../../resources/Icons/table.png");
        pushButtonTable->setIcon(iconTable);
        //pushButton->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum);
        pushButtonTable->setMaximumWidth(20);
        if (QString::fromStdString(resultitem.Name()).contains(":"))
            pushButtonTable->setObjectName(QString::fromStdString(resultitem.Name()));
        else
            pushButtonTable->setObjectName(QString::number(result_item_counter) + ":" + QString::fromStdString(resultitem.Name()));
        ui->gridLayout->addWidget(pushButtonTable,ui->gridLayout->rowCount()-1,3,1,1,Qt::AlignTop);
        connect(pushButtonTable,SIGNAL(clicked()),this,SLOT(on_result_table_clicked()));
    }

}


void ResultsWindow::on_result_graph_clicked()
{
    qDebug()<<sender()->objectName();
    GeneralChart *resultgraph = new GeneralChart(this);
    resultgraph->setWindowFlag(Qt::WindowMinMaxButtonsHint);
    resultgraph->setWindowTitle(QString::fromStdString(results->operator[](sender()->objectName().toStdString()).Name()));
    resultgraph->Plot(&results->operator[](sender()->objectName().toStdString()));
    resultgraph->show();
}


void ResultsWindow::on_result_table_clicked()
{
    qDebug()<<sender()->objectName();
    ResultTableViewer *tableviewer = new ResultTableViewer(this);
    tableviewer->setWindowFlag(Qt::WindowMinMaxButtonsHint);
    QTableWidget *tablewidget = results->operator[](sender()->objectName().toStdString()).Result()->ToTable();
    if (results->operator[](sender()->objectName().toStdString()).TableTitle() == "")
        tableviewer->setWindowTitle(QString::fromStdString(results->operator[](sender()->objectName().toStdString()).Name()));
    else
        tableviewer->setWindowTitle(QString::fromStdString(results->operator[](sender()->objectName().toStdString()).TableTitle()));
    tableviewer->SetTable(tablewidget);
    tableviewer->show();
}

void ResultsWindow::on_result_export_clicked()
{
    qDebug()<<sender()->objectName();
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save"), "",
        tr("text file (*.txt)"));

    if (!fileName.contains("."))
        fileName+=".txt";
    QFile file(fileName);

    file.open(QIODevice::WriteOnly | QIODevice::Text);
    if (!file.isOpen())
        return;
    
    results->operator[](sender()->objectName().toStdString()).Result()->writetofile(&file);

    file.close();
    
}
