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
#include "QMap"
#include "QJsonDocument"
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
    ui->frame->setVisible(false);
    QJsonDocument tools = loadJson(qApp->applicationDirPath()+"/../../resources/tools.json");
    QStandardItemModel *toolsmodel = ToQStandardItemModel(tools);
    ui->treeViewtools->setModel(toolsmodel);
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
    ui->treeView->setModel(ToQStandardItemModel(&data));
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
            elemental_profile_set = data.AppendSampleSet(sheetnames[sheetnumber].toStdString());
        }
        else
        {
            elemental_profile_set = data.AppendSampleSet(sheetnames[sheetnumber].toStdString());
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
    data.PopulateElementDistributions();
    data.AssignAllDistributions();
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
    ui->verticalLayout_3->addWidget(plotter);
    ui->frame->setVisible(true);
    ui->frame->setEnabled(true);

}

QStandardItemModel* MainWindow::ToQStandardItemModel(const SourceSinkData* srcsinkdata)
{
    if (columnviewmodel)
        delete columnviewmodel;
    columnviewmodel = new QStandardItemModel();

    vector<string> group_names = data.GroupNames();
    for (int i=0; i<group_names.size(); i++)
    {
        /* Create the phone groups as QStandardItems */
        QStandardItem *group = new QStandardItem(QString::fromStdString(group_names[i]));
        group->setData("Parent",Qt::UserRole);
        /* Append to each group 5 person as children */
        for (map<string,Elemental_Profile>::iterator it= data.sample_set(group_names[i])->begin();it!=data.sample_set(group_names[i])->end(); it++)
        {
            QStandardItem *child = new QStandardItem(QString::fromStdString(it->first));
            child->setData("Child",Qt::UserRole);
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
    vector<vector<string>> samples_selected;
    QModelIndexList indexes = ui->treeView->selectionModel()->selectedIndexes();
        for (int i=0; i<indexes.size(); i++) {
        {
            qDebug()<<indexes[i].data(Qt::UserRole).toString();
            if (indexes[i].data(Qt::UserRole).toString()=="Parent")
            {
                QString Group_Name_Selected = indexes[i].data().toString();
                vector<string> Sample_Names = data.sample_set(Group_Name_Selected.toStdString())->SampleNames();
                for (unsigned int sample_counter=0; sample_counter<Sample_Names.size(); sample_counter++)
                {
                    vector<string> item;
                    item.push_back(indexes[i].data().toString().toStdString());
                    item.push_back(indexes[i].child(sample_counter,0).data().toString().toStdString());
                    samples_selected.push_back(item);
                }
            }
            else
            {
                vector<string> item;
                item.push_back(indexes[i].parent().data().toString().toStdString());
                item.push_back(indexes[i].data().toString().toStdString());
                samples_selected.push_back(item);
            }
        }
    }

    profiles_data extracted_data = data.ExtractData(samples_selected);

    if (plotter==nullptr)
    {
        plotter = new GeneralPlotter(this);
        ui->verticalLayout_3->addWidget(plotter);

    }

    plotter->Clear();
    plotter->AddScatters(extracted_data.sample_names,extracted_data.element_names, extracted_data.values);
    plotter->SetYAxisScaleType(AxisScale::log);
    ui->frame->setVisible(true);
    ui->frame->setEnabled(true);

}

QJsonDocument MainWindow::loadJson(const QString &fileName) {
    QFile jsonFile(fileName);
    jsonFile.open(QFile::ReadOnly);
    return QJsonDocument().fromJson(jsonFile.readAll());
}

void MainWindow::saveJson(const QJsonDocument &document, const QString &fileName) {
    QFile jsonFile(fileName);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(document.toJson());
}

QStandardItem* MainWindow::ToQStandardItem(const QString &key, const QJsonObject &json)
{
    QStandardItem *standarditem = new QStandardItem(key);
    for (int i=0; i<json.keys().size(); i++)
    {
        if (json[json.keys()[i]].isObject())
        {
            standarditem->appendRow(ToQStandardItem(json.keys()[i],json[json.keys()[i]].toObject()));
        }
        else
        {
            QStandardItem *subitem = new QStandardItem(json.keys()[i]);
            standarditem->appendRow(subitem);
        }
    }
    return standarditem;
}

QStandardItemModel* MainWindow::ToQStandardItemModel(const QJsonDocument &jsondocument)
{
    QStandardItemModel *standarditemmodel = new QStandardItemModel();
    QJsonObject jsonobject = jsondocument.object();
    QStandardItem *standarditem = ToQStandardItem("Tools",jsonobject);
    standarditemmodel->appendRow(standarditem);
    return standarditemmodel;
}
