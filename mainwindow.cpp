#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QDebug"
#include "QFileDialog"
#include "QDir"
#include "sourcesinkdata.h"
#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"
#include "indicatesheetsdialog.h"
#include "plotwindow.h"
#include "QMessageBox"
#include "Utilities.h"
using namespace QXlsx;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionImport_Elemental_Profile_from_Excel,SIGNAL(triggered()),this,SLOT(on_import_excel()));
    connect(ui->actionRaw_Elemental_Profiles,SIGNAL(triggered()),this,SLOT(on_plot_raw_elemental_profiles()));
    connect(ui->actionTestGraphs, SIGNAL(triggered()),this,SLOT(on_test_plot()));
    ui->treeView->setSelectionMode(QAbstractItemView::MultiSelection);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_import_excel()
{
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open"), "",
            tr("Excel files (*.xlsx);; All files (*.*)"),nullptr,QFileDialog::DontUseNativeDialog);


    if (fileName!="")
    {
        ReadExcel(fileName);
    }
    ui->treeView->setModel(ToQStandardItemMode(&data));
    connect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection &)), this, SLOT(on_tree_selectionChanged(QItemSelection)));

}

void MainWindow::on_plot_raw_elemental_profiles()
{
    PlotWindow *plotwindow = new PlotWindow(this);
    plotwindow->show();
}

bool MainWindow::ReadExcel(const QString &filename)
{
    Document xlsxR(filename);
    QStringList sheetnames;
    if ( xlsxR.load() ) // load excel file
    {
        qDebug() << "[debug] success to load xlsx file.";
        sheetnames = xlsxR.sheetNames();
    }

    for (int i=0; i<sheetnames.count(); i++)
    {
        qDebug() << sheetnames[i];
    }

    IndicateSheetsDialog *indicatesheetdialog = new IndicateSheetsDialog(this);
    indicatesheetdialog->Populate_Table(sheetnames);
    indicatesheetdialog->exec();
    indicatesheetdialog->close();
    qDebug()<<"Sink Sheet is: " << Sink_Sheet;

    QList<QStringList> element_names;
    for (int sheetnumber=0; sheetnumber<sheetnames.count(); sheetnumber++)
    {
        xlsxR.selectSheet(sheetnames[sheetnumber]);
        QStringList elem_names;
        QString elem="none";
        int col = 2;
        while (elem!="" && xlsxR.cellAt(1,col))
        {
            elem = xlsxR.cellAt(1,col)->readValue().toString().trimmed();
            if (elem!="")
                elem_names.append(elem);
            qDebug()<<elem;
            col++;
        }
        if (sheetnumber>0)
        {   if (elem_names!=element_names.last())
            {
                QMessageBox::warning(this, "CMBSource", tr("Element names and their order in all sheets must be identical\n"), QMessageBox::Ok);
                WriteMessageOnScreen("Name of elements in sheet '" + sheetnames[sheetnumber-1] + "' and '" + sheetnames[sheetnumber] + "' are different",Qt::red);
                return false;
            }
        }
        element_names.append(elem_names);

    }

    for (int sheetnumber=0; sheetnumber<sheetnames.count(); sheetnumber++)
    {
        xlsxR.selectSheet(sheetnames[sheetnumber]);
        int row=2;
        Elemental_Profile_Set *elemental_profile_set = nullptr;
        if (sheetnumber==Sink_Sheet)
        {
            elemental_profile_set = data.Append_Target(sheetnames[sheetnumber].toStdString());
        }
        else
        {
            elemental_profile_set = data.Append_Source(sheetnames[sheetnumber].toStdString());
        }

        while (xlsxR.cellAt(row,1))
        {
            QString sample_name=xlsxR.cellAt(row,1)->readValue().toString();
            Elemental_Profile elemental_profile;
            for (int col=0; col<element_names[0].count(); col++)
            {
                double value = xlsxR.cellAt(row,col+2)->readValue().toDouble();
                elemental_profile.AppendElement(element_names[0][col].toStdString(),value);
            }
            row++;
            elemental_profile_set->Append_Profile(sample_name.toStdString(),elemental_profile);
        }

    }

    qDebug()<<"Reading element information done!";

    return true;
}

void MainWindow::WriteMessageOnScreen(const QString &text, QColor color)
{
    ui->textBrowser->setTextColor(color);
    ui->textBrowser->append(text);
}

void MainWindow::on_test_plot()
{
    plotter = new GeneralPlotter(this);
    vector<string> x;
    vector<vector<double>> y;

    for (int i=0; i<10; i++)
        x.push_back("case " + aquiutils::numbertostring(i));

    vector<string> names;
    for (int j=0; j<5; j++)
    {   vector<double> y_vec;
        for (int i=0; i<10; i++)
        {
            y_vec.push_back(sin(i)+0.1*j);
        }
        y.push_back(y_vec);
        names.push_back("Scenario: " + aquiutils::numbertostring(j));

    }
    plotter->AddScatters(names,x,y);
    ui->dockWidgetGraph->setWidget(plotter);
}

QStandardItemModel* MainWindow::ToQStandardItemMode(const SourceSinkData* srcsinkdata)
{
    if (columnviewmodel)
        delete columnviewmodel;
    columnviewmodel = new QStandardItemModel();

    vector<string> group_names = data.GroupNames();
    for (int i=0; i<group_names.size(); i++)
    {
        /* Create the phone groups as QStandardItems */
        QStandardItem *group = new QStandardItem(QString::fromStdString(group_names[i]));

        /* Append to each group 5 person as children */
        for (map<string,Elemental_Profile>::iterator it= data.sample_set(group_names[i])->begin();it!=data.sample_set(group_names[i])->end(); it++)
        {
            QStandardItem *child = new QStandardItem(QString::fromStdString(it->first));
            group->appendRow(child);
        }
        /* append group as new row to the model. model takes the ownership of the item */
        columnviewmodel->appendRow(group);
    }
    columnviewmodel->setColumnCount(1);
    QStringList columnTitles = QStringList() << "Groups";
    for (int i = 0; i < columnTitles.count(); ++i)
        columnviewmodel->setHeaderData(i, Qt::Horizontal, columnTitles.at(i), Qt::DisplayRole);
    return columnviewmodel;
}

void MainWindow::on_tree_selectionChanged(const QItemSelection &changed)
{
    qDebug()<<"Selection changed "<<changed;

    QModelIndexList indexes = ui->treeView->selectionModel()->selectedIndexes();
        for (int i=0; i<indexes.size(); i++) {
        qDebug()<<indexes[i].data();

    }
    plotter->Clear();

}
