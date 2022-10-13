#include "resultswindow.h"
#include "ui_resultswindow.h"
#include "QTextBrowser"
#include "QDebug"
#include "QPushButton"
#include "generalchart.h"

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
    QPushButton *pushButton = new QPushButton(this);
    QIcon iconGraph = QIcon(qApp->applicationDirPath()+"/../../resources/Icons/Graph.png");
    pushButton->setIcon(iconGraph);
    //pushButton->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum);
    pushButton->setMaximumWidth(20);
    pushButton->setObjectName(QString::fromStdString(resultitem.name));
    ui->gridLayout->addWidget(textBrowser,ui->gridLayout->rowCount(),0);
    ui->gridLayout->addWidget(pushButton,ui->gridLayout->rowCount()-1,1);
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
    int ht = textBrowser->fontMetrics().height();
    textBrowser->setMaximumHeight(textBrowser->fontMetrics().height() * (count+3));
    textBrowser->setMinimumHeight(textBrowser->fontMetrics().height() * (count+2));
    connect(pushButton,SIGNAL(clicked()),this,SLOT(on_result_clicked()));

}


void ResultsWindow::on_result_clicked()
{
    qDebug()<<sender()->objectName();
    GeneralChart *resultgraph = new GeneralChart(this);
    resultgraph->setWindowTitle(sender()->objectName());
    resultgraph->show();
}
