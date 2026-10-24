// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

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

void ResultsWindow::AppendResult(const ResultItem &resultitem, const std::string &key)
{

    result_item_counter++;
    // The buttons find their result by its key in the Results map. This used
    // to be rebuilt from the name, which went wrong for names containing a
    // colon ("Convergence: split R-hat"): the lookup missed, created an empty
    // result, and the table button crashed on it.
    const QString result_key = QString::fromStdString(key);
    QTextBrowser *textBrowser = new QTextBrowser(ui->scrollAreaWidgetContents);
    textBrowser->setObjectName(result_key);
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
        pushButtonGraph->setObjectName(result_key);

        ui->gridLayout->addWidget(pushButtonGraph,ui->gridLayout->rowCount()-1,1,1,1,Qt::AlignTop);
        connect(pushButtonGraph,SIGNAL(clicked()),this,SLOT(on_result_graph_clicked()));
    }

    QPushButton *pushButtonExport = new QPushButton(this);
    QIcon iconExport = QIcon(qApp->applicationDirPath()+"/../../resources/Icons/export.png");
    pushButtonExport->setIcon(iconExport);
    //pushButton->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum);
    pushButtonExport->setMaximumWidth(20);
    pushButtonExport->setObjectName(result_key);
    ui->gridLayout->addWidget(pushButtonExport,ui->gridLayout->rowCount()-1,2,1,1,Qt::AlignTop);
    connect(pushButtonExport,SIGNAL(clicked()),this,SLOT(on_result_export_clicked()));


    if (resultitem.ShowTable())
    {   QPushButton *pushButtonTable = new QPushButton(this);
        QIcon iconTable = QIcon(qApp->applicationDirPath()+"/../../resources/Icons/table.png");
        pushButtonTable->setIcon(iconTable);
        //pushButton->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum);
        pushButtonTable->setMaximumWidth(20);
        pushButtonTable->setObjectName(result_key);
        ui->gridLayout->addWidget(pushButtonTable,ui->gridLayout->rowCount()-1,3,1,1,Qt::AlignTop);
        connect(pushButtonTable,SIGNAL(clicked()),this,SLOT(on_result_table_clicked()));
    }

}


ResultItem* ResultsWindow::ResultForButton(QObject* button)
{
    if (results == nullptr || button == nullptr)
        return nullptr;
    // find(), not operator[]: a miss must not insert an empty result
    auto it = results->find(button->objectName().toStdString());
    if (it == results->end() || it->second.Result() == nullptr)
        return nullptr;
    return &it->second;
}

void ResultsWindow::on_result_graph_clicked()
{
    ResultItem* item = ResultForButton(sender());
    if (item == nullptr)
        return;
    GeneralChart *resultgraph = new GeneralChart(this);
    resultgraph->setWindowFlag(Qt::WindowMinMaxButtonsHint);
    resultgraph->setWindowTitle(QString::fromStdString(item->Name()));
    resultgraph->Plot(item);
    resultgraph->show();
}


void ResultsWindow::on_result_table_clicked()
{
    ResultItem* item = ResultForButton(sender());
    if (item == nullptr)
        return;
    ResultTableViewer *tableviewer = new ResultTableViewer(this);
    tableviewer->setWindowFlag(Qt::WindowMinMaxButtonsHint);
    QTableWidget *tablewidget = item->Result()->ToTable();
    if (item->TableTitle() == "")
        tableviewer->setWindowTitle(QString::fromStdString(item->Name()));
    else
        tableviewer->setWindowTitle(QString::fromStdString(item->TableTitle()));
    tableviewer->SetTable(tablewidget);
    tableviewer->show();
}

void ResultsWindow::on_result_export_clicked()
{
    ResultItem* item = ResultForButton(sender());
    if (item == nullptr)
        return;
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save"), "",
        tr("text file (*.txt)"));

    if (!fileName.contains("."))
        fileName+=".txt";
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    
    item->Result()->writetofile(&file);

    file.close();
    
}
