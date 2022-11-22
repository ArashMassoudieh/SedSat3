#include "resultswindow.h"
#include "ui_resultswindow.h"
#include "QTextBrowser"
#include "QDebug"
#include "QPushButton"
#include "generalchart.h"

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

void ResultsWindow::AppendResult(const result_item &resultitem)
{

    QTextBrowser *textBrowser = new QTextBrowser(ui->scrollAreaWidgetContents);
    textBrowser->setObjectName(QString::fromStdString(resultitem.name));
    //textBrowser->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
    QPushButton *pushButtonGraph = new QPushButton(this);
    QIcon iconGraph = QIcon(qApp->applicationDirPath()+"/../../resources/Icons/Graph.png");
    pushButtonGraph->setIcon(iconGraph);
    //pushButton->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum);
    pushButtonGraph->setMaximumWidth(20);
    pushButtonGraph->setObjectName(QString::fromStdString(resultitem.name));

    QPushButton *pushButtonExport = new QPushButton(this);
    QIcon iconExport = QIcon(qApp->applicationDirPath()+"/../../resources/Icons/export.png");
    pushButtonExport->setIcon(iconExport);
    //pushButton->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum);
    pushButtonExport->setMaximumWidth(20);
    pushButtonExport->setObjectName(QString::fromStdString(resultitem.name));

    ui->gridLayout->addWidget(textBrowser,ui->gridLayout->rowCount(),0);
    ui->gridLayout->addWidget(pushButtonGraph,ui->gridLayout->rowCount()-1,1);
    ui->gridLayout->addWidget(pushButtonExport,ui->gridLayout->rowCount()-1,2);
    textBrowser->setTextColor(Qt::red);
    textBrowser->append(QString::fromStdString(resultitem.name)+":");
    textBrowser->setTextColor(Qt::black);
    textBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QString str = QString::fromStdString(resultitem.result->ToString());
    textBrowser->append(str);
    int count = 0;
    for(int i = 0;i < str.length();i++)
        if(str.at(i).cell() == '\n')
            count++;
    textBrowser->setMaximumHeight(textBrowser->fontMetrics().height() * (count+3));
    textBrowser->setMinimumHeight(textBrowser->fontMetrics().height() * (count+2));
    connect(pushButtonGraph,SIGNAL(clicked()),this,SLOT(on_result_graph_clicked()));
    connect(pushButtonExport,SIGNAL(clicked()),this,SLOT(on_result_export_clicked()));

}


void ResultsWindow::on_result_graph_clicked()
{
    qDebug()<<sender()->objectName();
    GeneralChart *resultgraph = new GeneralChart(this);
    resultgraph->setWindowTitle(sender()->objectName());
    resultgraph->Plot(&results->operator[](sender()->objectName().toStdString()));
    resultgraph->show();
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
    
    results->operator[](sender()->objectName().toStdString()).result->writetofile(&file);

    file.close();
    
}
