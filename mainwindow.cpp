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
#include "formelementinformation.h"
#include "elementstablemodel.h"
#include "elementtabledelegate.h"
#include "GA.h"
#include "genericform.h"
#include "QMessageBox"
#include "ProgressWindow.h"
#include "resultswindow.h"
#include "aboutdialog.h"
#include "resultsetitem.h"
#include "resultitem.h"
#include "selectsamples.h"
//#include "MCMC.h"

#ifdef _WIN32
#include <windows.h>
#include <ShlObj.h>
#pragma comment(lib "shell32.lib")
#endif

#define RECENT "SedSatrecentFiles.txt"

#ifndef max_num_recent_files
#define max_num_recent_files 15
#endif


#define version "1.0.1"
using namespace QXlsx;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QIcon mainicon(qApp->applicationDirPath()+"/../../resources/Icons/CMBSource_Icon.png");
    conductor = new Conductor(this);
    setWindowIcon(mainicon);
    connect(ui->actionImport_Elemental_Profile_from_Excel,SIGNAL(triggered()),this,SLOT(on_import_excel()));
    connect(ui->actionRaw_Elemental_Profiles,SIGNAL(triggered()),this,SLOT(on_plot_raw_elemental_profiles()));
    connect(ui->actionTestGraphs, SIGNAL(triggered()),this,SLOT(on_test_plot()));
    ui->treeView->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->frame->setVisible(false);
    qDebug()<<qApp->applicationDirPath()+"/../../resources/tools.json";
    //tools
    qDebug()<<qApp->applicationDirPath()+"/../../resources/tools.json";
    QJsonDocument tools = loadJson(qApp->applicationDirPath()+"/../../resources/tools.json");
    formsstructure = loadJson(qApp->applicationDirPath()+"/../../resources/forms_structures.json");
    QStandardItemModel *toolsmodel = ToQStandardItemModel(tools);
    toolsmodel->setHorizontalHeaderLabels(QStringList()<<"Tools");
    ui->treeViewtools->setModel(toolsmodel);
    ui->treeViewtools->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //results
    resultsviewmodel = new QStandardItemModel();
    ui->TreeView_Results->setModel(resultsviewmodel);
    resultsviewmodel->setHorizontalHeaderLabels(QStringList()<<"Results");

    DeleteAction = new QAction("Delete",ResultscontextMenu);
    ResultscontextMenu = new QMenu(ui->TreeView_Results);
    ResultscontextMenu->addAction(DeleteAction);
    connect(DeleteAction, SIGNAL(triggered()), this, SLOT(DeleteResults()));


    ui->TreeView_Results->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->TreeView_Results, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));
    connect(ui->actionTestLevenberg_Marquardt, SIGNAL(triggered()), this, SLOT(on_TestLevenberg_Marquardt()));
    connect(ui->actionConstituent_Properties,SIGNAL(triggered()),this,SLOT(on_constituent_properties_triggered()));
    connect(ui->actionInclude_Exclude_Samples,SIGNAL(triggered()),this,SLOT(onIncludeExcludeSample()));
    connect(ui->actionOrganic_Matter_Size_correction,SIGNAL(triggered()),this,SLOT(onOMSizeCorrection()));
    connect(ui->actionTestDialog,SIGNAL(triggered()),this,SLOT(on_test_dialog_triggered()));
    connect(ui->treeViewtools,SIGNAL(doubleClicked(const QModelIndex&)),this, SLOT(on_tool_executed(const QModelIndex&)));
    connect(ui->TreeView_Results,SIGNAL(doubleClicked(const QModelIndex&)),this, SLOT(on_old_result_requested(const QModelIndex&)));
    connect(ui->actionTestLikelihoods, SIGNAL(triggered()),this,SLOT(on_test_likelihood()));
    connect(ui->actionTestProgressGraph, SIGNAL(triggered()), this, SLOT(on_test_progress_window()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(onAboutTriggered()));
    connect(ui->actionSave_Project,SIGNAL(triggered()),this,SLOT(onSaveProject()));
    connect(ui->actionSave_As,SIGNAL(triggered()),this,SLOT(onSaveProjectAs()));
    connect(ui->actionOpen_Project,SIGNAL(triggered()),this,SLOT(onOpenProject()));
    CGA<SourceSinkData> GA;
    centralform.reset(ui->textBrowser);
    conductor->SetWorkingFolder(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    readRecentFilesList();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSaveProject()
{
    QString fileName;
    if (!ProjectFileName.isEmpty())
        fileName = ProjectFileName;
    else
    {   fileName = QFileDialog::getSaveFileName(this,
            tr("Save"), "",
            tr("CMB Source file (*.cmb);; All files (*.*)"),nullptr,QFileDialog::DontUseNativeDialog);
        addToRecentFiles(fileName,true);
    }

    if (fileName.isEmpty())
        return;

    if (!fileName.toLower().contains(".cmb"))
        fileName+=".cmb";
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    QFileInfo fi(file);
    conductor->SetWorkingFolder(fi.absolutePath());

    QJsonObject outputjsonobject;
    outputjsonobject["Element Information"] = Data()->ElementInformationToJsonObject();
    outputjsonobject["Element Data"] = Data()->ElementDataToJsonObject();
    outputjsonobject["Target Group"] = QString::fromStdString(Data()->TargetGroup());

    QJsonObject resultsetjsonobject;
    for (int i=0; i<resultsviewmodel->rowCount(); i++)
    {
        QModelIndex index = resultsviewmodel->index(i,0);
        Results *resultset = static_cast<ResultSetItem*>(resultsviewmodel->item(index.row()))->result;
        resultsetjsonobject[static_cast<ResultSetItem*>(resultsviewmodel->item(index.row()))->text()] = resultset->toJsonObject();
    }
    outputjsonobject["Results"] = resultsetjsonobject;
    QJsonDocument outputjsondocument(outputjsonobject);

    file.write(outputjsondocument.toJson());
    file.close();
    
}

void MainWindow::onSaveProjectAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save"), "",
            tr("CMB Source file (*.cmb);; All files (*.*)"),nullptr,QFileDialog::DontUseNativeDialog);

    if (fileName.isEmpty())
        return;


    if (!fileName.toLower().contains(".cmb"))
        fileName+=".cmb";
    addToRecentFiles(fileName,true);
    ProjectFileName = fileName;
    this->setWindowTitle("SetSat3:" + ProjectFileName);
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    QFileInfo fi(file);
    conductor->SetWorkingFolder(fi.absolutePath());

    QJsonObject outputjsonobject;
    outputjsonobject["Element Information"] = Data()->ElementInformationToJsonObject();
    outputjsonobject["Element Data"] = Data()->ElementDataToJsonObject();
    outputjsonobject["Target Group"] = QString::fromStdString(Data()->TargetGroup());

    QJsonObject resultsetjsonobject;
    for (int i=0; i<resultsviewmodel->rowCount(); i++)
    {
        QModelIndex index = resultsviewmodel->index(i,0);
        Results *resultset = static_cast<ResultSetItem*>(resultsviewmodel->item(index.row()))->result;
        resultsetjsonobject[static_cast<ResultSetItem*>(resultsviewmodel->item(index.row()))->text()] = resultset->toJsonObject();
    }
    outputjsonobject["Results"] = resultsetjsonobject;
    QJsonDocument outputjsondocument(outputjsonobject);

    file.write(outputjsondocument.toJson());
    file.close();

}


void MainWindow::onOpenProject()
{
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open"), "",
            tr("CMB Source file (*.cmb);; All files (*.*)"),nullptr,QFileDialog::DontUseNativeDialog);

    if (fileName.isEmpty())
        return;


    if (LoadModel(fileName))
        addToRecentFiles(fileName,true);
}

bool MainWindow::LoadModel(const QString &fileName)
{
    ProjectFileName = fileName;

    this->setWindowTitle("SetSat3:" + ProjectFileName);
    QFile file(fileName);
    QFileInfo fi(file);
    conductor->SetWorkingFolder(fi.absolutePath());
    if (file.open(QIODevice::ReadOnly))
    {
        Data()->ReadFromFile(&file);
        file.close();
        QFile fileres(fileName);
        fileres.open(QIODevice::ReadOnly);
        QJsonObject jsondoc = QJsonDocument().fromJson(fileres.readAll()).object();
        QJsonObject resultsjson = jsondoc["Results"].toObject();
        resultsviewmodel->clear();
        resultsviewmodel->setHorizontalHeaderLabels(QStringList()<<"Results");
        for(QString key: resultsjson.keys() ) {

            ResultSetItem *resultset = new ResultSetItem(key);
            resultset->setToolTip(key);
            resultset->result = new Results();
            resultset->result->ReadFromJson(resultsjson.value(key).toObject());
            resultsviewmodel->appendRow(resultset);
         }


        file.close();
        DataCollection.PopulateElementDistributions();
        DataCollection.AssignAllDistributions();
        InitiateTables();
        return true;
    }
    return false;
}

void MainWindow::on_import_excel()
{
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open"), "",
            tr("Excel files (*.xlsx);; All files (*.*)"),nullptr,QFileDialog::DontUseNativeDialog);

    QFileInfo fi(fileName);
    
    conductor->SetWorkingFolder(fi.absolutePath());
    if (fileName!="")
    {
        ReadExcel(fileName);
    }

    InitiateTables();
    QMessageBox::information(this,"Exclude Elements","To not include in analysis, double click, and uncheck box for the constituent", QMessageBox::Ok);
    on_constituent_properties_triggered();

}

void MainWindow::InitiateTables()
{
    QList<QStandardItem*> modelitems;
    QList<QStandardItem*> elementitems;
    if (columnviewmodel)
        delete columnviewmodel;
    columnviewmodel = new QStandardItemModel();
    columnviewmodel->setColumnCount(1);
    modelitems.append(ToQStandardItem("Samples",&DataCollection));
    columnviewmodel->appendRow(modelitems);
    elementitems.append(ElementsToQStandardItem("Elements",&DataCollection));
    columnviewmodel->appendRow(elementitems);
    columnviewmodel->setHorizontalHeaderLabels(QStringList()<<"Data");
    ui->treeView->setModel(columnviewmodel);
    connect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection &)), this, SLOT(on_tree_selectionChanged(QItemSelection)));
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(preparetreeviewMenu(const QPoint &)));
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
            elemental_profile_set = DataCollection.AppendSampleSet(sheetnames[sheetnumber].toStdString());
        }
        else
        {
            elemental_profile_set = DataCollection.AppendSampleSet(sheetnames[sheetnumber].toStdString());
        }

        while (xlsxR.cellAt(row,1) && xlsxR.cellAt(row,1)->readValue().toString()!="")
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
    DataCollection.PopulateElementInformation();
    DataCollection.PopulateElementDistributions();
    DataCollection.AssignAllDistributions();
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
    map<string,vector<double>> _data;

    for (int i=0; i<10; i++)
    {
        vector<double> y_val(i+1);
        for (unsigned int j=0; j<y_val.size(); j++)
        {
            y_val[j]=i+0.2*j;
        }
        _data["case " + aquiutils::numbertostring(i)] = y_val;
    }
    plotter->AddNoneUniformScatter(_data,1);
    ui->verticalLayout_3->addWidget(plotter);
    ui->frame->setVisible(true);
    ui->frame->setEnabled(true);

}

QStandardItemModel* MainWindow::ToQStandardItemModel(const SourceSinkData* srcsinkdata)
{
    if (columnviewmodel)
        delete columnviewmodel;
    columnviewmodel = new QStandardItemModel();

    vector<string> group_names = DataCollection.GroupNames();
    for (int i=0; i<group_names.size(); i++)
    {
        /* Create the phone groups as QStandardItems */
        QStandardItem *group = new QStandardItem(QString::fromStdString(group_names[i]));
        group->setData("Parent",Qt::UserRole);
        /* Append to each group 5 person as children */
        for (map<string,Elemental_Profile>::iterator it= DataCollection.sample_set(group_names[i])->begin();it!=DataCollection.sample_set(group_names[i])->end(); it++)
        {
            QStandardItem *child = new QStandardItem(QString::fromStdString(it->first));
            child->setData(QString::fromStdString(it->first), elementRole );
            child->setData(QString::fromStdString(group_names[i]), groupRole );
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

QStandardItem* MainWindow::ToQStandardItem(const QString &key, const SourceSinkData* srcsinkdata)
{

    QStandardItem *columnviewitem = new QStandardItem(key);
    columnviewitem->setData("RootItem",Qt::UserRole);
    vector<string> group_names = DataCollection.GroupNames();
    for (int i=0; i<group_names.size(); i++)
    {

        QStandardItem *group = new QStandardItem(QString::fromStdString(group_names[i]));
        group->setData("Group",Qt::UserRole);
        group->setData(QString::fromStdString(group_names[i]),groupRole);
        for (map<string,Elemental_Profile>::iterator it= DataCollection.sample_set(group_names[i])->begin();it!=DataCollection.sample_set(group_names[i])->end(); it++)
        {
            QStandardItem *child = new QStandardItem(QString::fromStdString(it->first));
            child->setData("Sample",Qt::UserRole);
            child->setData(QString::fromStdString(group_names[i]),groupRole);
            child->setData(QString::fromStdString(it->first),sampleRole);

            group->appendRow(child);
        }
        /* append group as new row to the model. model takes the ownership of the item */
        columnviewitem->appendRow(group);
    }
    return columnviewitem;
}

QStandardItem* MainWindow::ElementsToQStandardItem(const QString &key, const SourceSinkData* srcsinkdata)
{
    QStandardItem *columnviewitem = new QStandardItem(key);
    columnviewitem->setData("RootItem",Qt::UserRole);
    vector<string> group_names = DataCollection.GroupNames();
    vector<string> element_names = DataCollection.ElementNames();
    for (unsigned int elem_counter=0; elem_counter<element_names.size(); elem_counter++)
    {   QStandardItem *element_item=new QStandardItem(QString::fromStdString(element_names[elem_counter]));
        element_item->setData("Element",Qt::UserRole);
        element_item->setData(QString::fromStdString(element_names[elem_counter]),elementRole);
        for (unsigned int group_counter=0; group_counter<group_names.size(); group_counter++)
        {
            QStandardItem *group = new QStandardItem(QString::fromStdString(group_names[group_counter]));
            group->setData("GroupInElements",Qt::UserRole);
            group->setData(QString::fromStdString(element_names[elem_counter]),elementRole);
            group->setData(QString::fromStdString(group_names[group_counter]),groupRole);

            element_item->appendRow(group);
        }
        columnviewitem->appendRow(element_item);
    }
    return columnviewitem;
}


void MainWindow::on_tree_selectionChanged(const QItemSelection &changed)
{
    qDebug()<<"Selection changed "<<changed;
    if (treeitemchangedprogramatically) return;
    if (plotter==nullptr)
    {
        plotter = new GeneralPlotter(this);
        ui->verticalLayout_3->addWidget(plotter);

    }
    plotter->Clear();
    enum class plottype {elemental_profiles, element_scatters} PlotType = plottype::elemental_profiles;
    vector<vector<string>> samples_selected;
    QModelIndexList indexes = ui->treeView->selectionModel()->selectedIndexes();
    if (changed.size()>0)
    {   QModelIndex changed_index = changed.at(0).indexes()[0];
        if (indexes.size()>0)
        {   if (indexes.contains(changed.at(0).indexes()[0]))
                if (changed.at(0).indexes()[0].data(Qt::UserRole)!=SelectedTreeItemType && SelectedTreeItemType!="None")
                {
                    SelectedTreeItemType = changed.at(0).indexes()[0].data(Qt::UserRole).toString();
                    treeitemchangedprogramatically=true;
                    ui->treeView->selectionModel()->clearSelection();
                    ui->treeView->selectionModel()->select(changed_index,QItemSelectionModel::Select | QItemSelectionModel::Rows);
                    treeitemchangedprogramatically=false;
                    indexes = ui->treeView->selectionModel()->selectedIndexes();
                }
        }
    }
    for (int i=0; i<indexes.size(); i++)
    {
        qDebug()<<indexes[i].data(Qt::UserRole).toString();
        if (indexes[i].data(Qt::UserRole).toString()=="Group")
        {
            QString Group_Name_Selected = indexes[i].data().toString();
            vector<string> Sample_Names = DataCollection.sample_set(Group_Name_Selected.toStdString())->SampleNames();
            for (int sample_counter=0; sample_counter<Sample_Names.size(); sample_counter++)
            {
                vector<string> item;
                item.push_back(indexes[i].data().toString().toStdString());
                item.push_back(ui->treeView->selectionModel()->model()->index(sample_counter,0, indexes[i]).data().toString().toStdString());
                samples_selected.push_back(item);
            }
        }
        else if (indexes[i].data(Qt::UserRole).toString()=="Sample")
        {
            vector<string> item;
            item.push_back(indexes[i].parent().data().toString().toStdString());
            item.push_back(indexes[i].data().toString().toStdString());
            samples_selected.push_back(item);
        }
        if (indexes[i].data(Qt::UserRole).toString()=="Element")
        {
            qDebug()<<"Element selected!";
            PlotType = plottype::element_scatters;
            QString Element_Name_Selected = indexes[i].data().toString();
            map<string,vector<double>> extracted_data = DataCollection.ExtractElementData(Element_Name_Selected.toStdString());
            qDebug()<<"Data extracted!";
            if (plotter==nullptr)
            {
                plotter = new GeneralPlotter(this);
                ui->verticalLayout_3->addWidget(plotter);

            }

            plotter->AddNoneUniformScatter(extracted_data,i);
            plotter->SetYAxisScaleType(AxisScale::log);
        }
    }


    if (samples_selected.size()>0 && PlotType==plottype::elemental_profiles)
    {   profiles_data extracted_data = DataCollection.ExtractData(samples_selected);
        plotter->AddScatters(extracted_data.sample_names,extracted_data.element_names, extracted_data.values);
        plotter->SetYAxisScaleType(AxisScale::log);
    }
    ui->frame->setVisible(true);
    ui->frame->setEnabled(true);
    SelectedTreeItemType = TreeQStringSelectedType();
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
            standarditem->setToolTip(json.keys()[i]);
        }
        else
        {
            QStandardItem *subitem = new QStandardItem(json.keys()[i]);
            subitem->setData(json.value(json.keys()[i]).toString(),Qt::UserRole);
            subitem->setToolTip(json.keys()[i]);
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

QString MainWindow::TreeQStringSelectedType()
{
    if (ui->treeView->selectionModel()->selectedIndexes().size()>0)
        return ui->treeView->selectionModel()->selectedIndexes()[0].data(Qt::UserRole).toString();
    else
        return "none";
}

void MainWindow::preparetreeviewMenu(const QPoint &pos)
{
    menu.reset(new QMenu(this));
    QTreeView *tree = ui->treeView;

    QModelIndex index = tree->currentIndex();
    qDebug()<<index.data(elementRole);
    qDebug()<<index.data(groupRole);
    qDebug()<<index.data(sampleRole);
    if (!index.data(elementRole).toString().isEmpty())
    {   QAction* showdistributions = menu->addAction("Show fitted distributions");
        QStringList RoleData;

        RoleData<< "Element=" + index.data(elementRole).toString();
        RoleData<< "Group=" + index.data(groupRole).toString();
        RoleData<<"Sample=" +  index.data(sampleRole).toString();
        RoleData<<"Action=SFD";
        showdistributions->setData(RoleData);
        connect(showdistributions, SIGNAL(triggered()), this, SLOT(showdistributionsforelements()));
    }



    if (menu)
        menu->exec( tree->mapToGlobal(pos) );
    return;

}

void MainWindow::showdistributionsforelements()
{
    QAction* act = qobject_cast<QAction*>(sender());
    QStringList keysStringList = act->data().toStringList();
    QMap<QString,QString> keys;
    for (unsigned int i=0; i<keysStringList.count(); i++)
    {
        keys[keysStringList[i].split("=")[0]] = keysStringList[i].split("=")[1];
    }

    QString Element = keys["Element"];
    QString Group = keys["Group"];
    QString Sample = keys["Sample"];
    QString Action = keys["Action"];


    if (Action == "SFD")
    {   PlotWindow *plotwindow = new PlotWindow(this);
        if (Group=="")
        {
            CTimeSeries<double> elem_dist = DataCollection.FittedDistribution(Element.toStdString())->EvaluateAsTimeSeries();
            plotwindow->Plotter()->AddTimeSeries("All samples", elem_dist.tToStdVector(),elem_dist.ValuesToStdVector());
            for (map<string,Elemental_Profile_Set>::iterator it=DataCollection.begin(); it!=DataCollection.end(); it++)
            {
                elem_dist = DataCollection.sample_set(it->first)->ElementalDistribution(Element.toStdString())->FittedDistribution()->EvaluateAsTimeSeries();
                plotwindow->Plotter()->AddTimeSeries(it->first, elem_dist.tToStdVector(),elem_dist.ValuesToStdVector());
            }

        }
        else
        {
            CTimeSeries<double> elem_dist = DataCollection.sample_set(Group.toStdString())->ElementalDistribution(Element.toStdString())->FittedDistribution()->EvaluateAsTimeSeries();
            plotwindow->Plotter()->AddTimeSeries((Group+":"+Element).toStdString(), elem_dist.tToStdVector(),elem_dist.ValuesToStdVector());
        }
        plotwindow->setWindowTitle(Element);
        plotwindow->Plotter()->SetLegend(true);
        plotwindow->show();
    }


}

void MainWindow::on_constituent_properties_triggered()
{

    FormElementInformation *formelems = new FormElementInformation(this);
    ElementTableModel *elementtablemodel = new ElementTableModel(&DataCollection,this);
    formelems->table()->setModel(elementtablemodel);
    ElementTableDelegate *elemDelegate = new ElementTableDelegate(&DataCollection, this);
    formelems->table()->setItemDelegate(elemDelegate);
    ui->verticalLayout_middle->addWidget(formelems);
    centralform.reset(formelems);
}

void MainWindow::onIncludeExcludeSample()
{
    Data()->OutlierAnalysisForAll(-3,3);
    Data()->BracketTest();
    SelectSamples *include_exclude_samples = new SelectSamples(this);
    include_exclude_samples->SetMode(mode::samples);
    include_exclude_samples->SetData(&DataCollection);
    ui->verticalLayout_middle->addWidget(include_exclude_samples);
    centralform.reset(include_exclude_samples);

}

void MainWindow::onOMSizeCorrection()
{
    if (DataCollection.OMandSizeConstituents()[0]=="" && DataCollection.OMandSizeConstituents()[1]=="")
    {
        QMessageBox::warning(this, "OpenHydroQual",tr("Perform Organic Matter and Size Correction first!\n"), QMessageBox::Ok);
            return;
    }
    SelectSamples *include_exclude_samples = new SelectSamples(this);
    include_exclude_samples->SetMode(mode::regressions);
    include_exclude_samples->SetData(&DataCollection);
    ui->verticalLayout_middle->addWidget(include_exclude_samples);
    centralform.reset(include_exclude_samples);

}


void MainWindow::on_test_dialog_triggered()
{

    QJsonObject mainjsonobject = formsstructure.object();
    QJsonObject GA_object = mainjsonobject.value("GA").toObject();
    GenericForm *form = new GenericForm(&GA_object,this,this);
    ui->verticalLayout_middle->addWidget(form);
    centralform.reset(form);
}

void MainWindow::on_tool_executed(const QModelIndex &index)
{
    qDebug()<<"Tool executed"<<index.data()<<": "<<index.data(Qt::UserRole);
    if (DataCollection.ElementNames().size()==0)
    {   QMessageBox::warning(this, "OpenHydroQual",tr("No data has been loaded!\n"), QMessageBox::Ok);
        return;
    }
    QJsonObject mainjsonobject = formsstructure.object();
    if (mainjsonobject.contains(index.data(Qt::UserRole).toString()))
    {   QJsonObject GA_object = mainjsonobject.value(index.data(Qt::UserRole).toString()).toObject();
        centralform.reset(new GenericForm(&GA_object,this, this));
        static_cast<GenericForm*>(centralform.get())->SetCommand(index.data(Qt::UserRole).toString());
        ui->verticalLayout_middle->addWidget(centralform.get());
    }



}

void MainWindow::on_old_result_requested(const QModelIndex& index)
{
    ResultsWindow *reswind = new ResultsWindow(this);
    Results *resultset = static_cast<ResultSetItem*>(resultsviewmodel->item(index.row()))->result;
    reswind->SetResults(static_cast<ResultSetItem*>(resultsviewmodel->item(index.row()))->result);
    reswind->setWindowTitle(static_cast<ResultSetItem*>(resultsviewmodel->item(index.row()))->text());
    for (map<string,ResultItem>::iterator it=resultset->begin(); it!=resultset->end(); it++)
    {
        reswind->AppendResult(it->second);
    }
    reswind->show();

}

bool MainWindow::Execute(const string &command, map<string,string> arguments)
{
    conductor->SetData(&DataCollection);
    bool outcome = conductor->Execute(command,arguments);
    if (outcome)
    {   ResultsWindow *reswind = new ResultsWindow(this);
        reswind->SetResults(conductor->GetResults());
        ResultSetItem *resultset = new ResultSetItem(QString::fromStdString(conductor->GetResults()->GetName()) + "_" + QDateTime::currentDateTime().toString(Qt::TextDate));
        resultset->setToolTip(QString::fromStdString(conductor->GetResults()->GetName()) + "_" + QDateTime::currentDateTime().toString(Qt::TextDate));
        resultset->result = conductor->GetResults();
        resultsviewmodel->appendRow(resultset);
        for (map<string,ResultItem>::iterator it=resultset->result->begin(); it!=resultset->result->end(); it++)
        {
            reswind->AppendResult(it->second);
        }
        reswind->setWindowTitle(QString::fromStdString(conductor->GetResults()->GetName()) + "_" + QDateTime::currentDateTime().toString(Qt::TextDate));
        reswind->show();
    }
    return outcome;

}

void MainWindow::on_test_likelihood()
{
    vector<string> elements = DataCollection.ElementsToBeUsedInCMB();
    vector<string> sources = DataCollection.SourceGroupNames();
    DataCollection.InitializeParametersObservations(DataCollection.SelectedTargetSample());
    DataCollection.SetParameterValue(0, 0.25);
    DataCollection.SetParameterValue(1, 0.25);
    DataCollection.SetParameterValue(2, 0.25);

    for (unsigned int element_counter=0; element_counter<elements.size(); element_counter++)
    {
        for (unsigned int source_group_counter=0; source_group_counter<sources.size(); source_group_counter++)
        {
            DataCollection.SetParameterValue(3+element_counter*sources.size()+source_group_counter,DataCollection.GetElementDistribution(elements[element_counter],sources[source_group_counter])->FittedDistribution()->parameters[0]);
        }
    }
    for (unsigned int element_counter=0; element_counter<elements.size(); element_counter++)
    {
        for (unsigned int source_group_counter=0; source_group_counter<sources.size(); source_group_counter++)
        {
            DataCollection.SetParameterValue(3+source_group_counter+element_counter*sources.size()+sources.size()*elements.size(),DataCollection.GetElementDistribution(elements[element_counter],sources[source_group_counter])->FittedDistribution()->parameters[1]);
        }
    }
    DataCollection.SetParameterValue(3+2*sources.size()*elements.size(),1);
    DataCollection.SetSelectedTargetSample("CTAIL3");
    cout<<DataCollection.LogLikelihood()<<std::endl;
}

void MainWindow::on_test_progress_window()
{
    ProgressWindow* prgwindow = new ProgressWindow(this);
    prgwindow->show(); 
    prgwindow->SetProgress(0.5);
    prgwindow->AppendPoint(0.1, 0.1);
    prgwindow->AppendPoint(0.2, 0.05);
    prgwindow->AppendPoint(0.3, 0.25);
    prgwindow->SetXRange(0, 2);
    prgwindow->SetYRange(0, 2);

}

void MainWindow::on_TestLevenberg_Marquardt()
{

}

void MainWindow::onAboutTriggered()
{
    AboutDialog* abtdlg = new AboutDialog(this);
    abtdlg->AppendText(QString("CMBSource version ") + QString(version));
    abtdlg->exec(); 
}

void MainWindow::onCustomContextMenu(const QPoint &point)
{
    indexresultselected = ui->TreeView_Results->indexAt(point);
        if (indexresultselected.isValid()) {
            ResultscontextMenu->exec(ui->TreeView_Results->viewport()->mapToGlobal(point));
        }
}
void MainWindow::DeleteResults()
{
    resultsviewmodel->removeRow(indexresultselected.row());
}

void MainWindow::readRecentFilesList()
{
//	//qDebug() << localAppFolderAddress();
//	QString add = localAppFolderAddress();
    ifstream file(localAppFolderAddress().toStdString()+RECENT);
    int count = 0;
    if (file.good())
    {
        string line;
        while (!file.eof())
        {
            getline(file, line);
            count++;
        }
        file.close();
    }

    file.open(localAppFolderAddress().toStdString() + RECENT);
    int n = 0;
    if (file.good())
    {
        string line;
        while (!file.eof())
        {
            getline(file, line);
            n++;
            QString fileName = QString::fromStdString(line);
            //qDebug() << fileName; QString::fromStdString(line);
            if (n>count-max_num_recent_files)
                addToRecentFiles(fileName, false);

        }
        file.close();

    }
}
void MainWindow::addToRecentFiles(QString fileName, bool addToFile)
{
    bool rewriteFile = false;
    if (recentFiles.contains(fileName) && fileName.trimmed() != "")
        if (recentFiles.indexOf(fileName) != recentFiles.count()-1)
        {
            ui->menuRecent->removeAction(ui->menuRecent->actions()[recentFiles.size() - 1 - recentFiles.indexOf(fileName)]);
            recentFiles.removeOne(fileName);
            addToFile = false;
            rewriteFile = true;
        }

    if (!recentFiles.contains(fileName) && fileName.trimmed() != "")
    {
        recentFiles.append(fileName);
        //		QAction * a = ui->menuRecent->addAction(fileName);// , this, SLOT(recentItem()));
        QAction * fileNameAction = new QAction(fileName, nullptr);
        if (ui->menuRecent->actions().size())
            ui->menuRecent->insertAction(ui->menuRecent->actions()[0], fileNameAction);
        else
            ui->menuRecent->addAction(fileNameAction);
        QObject::connect(fileNameAction, SIGNAL(triggered()), this, SLOT(on_actionRecent_triggered()));

        if (addToFile)
        {
            CreateFileIfDoesNotExist(localAppFolderAddress() + RECENT);
            ofstream file(localAppFolderAddress().toStdString() + RECENT, fstream::app);
            if (file.good())
                file << fileName.toStdString() << std::endl;
            file.close();
        }
        if (rewriteFile)
            writeRecentFilesList();
    }
}

void MainWindow::writeRecentFilesList()
{
    ofstream file(localAppFolderAddress().toStdString() + RECENT);
    if (file.good())
    {
        foreach (QString fileName , recentFiles)
            file << fileName.toStdString() << std::endl;
    }
    file.close();
}

QString localAppFolderAddress() {
    #ifdef _WIN32
    TCHAR szPath[MAX_PATH];

    if (SUCCEEDED(SHGetFolderPath(NULL,
        CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE,
        NULL,
        0,
        szPath)))
    {
        return QString("%1/").arg(QString::fromStdWString(szPath));
        //PathAppend(szPath, TEXT("New Doc.txt"));
        //HANDLE hFile = CreateFile(szPath, ...);
    }
#else
    return QString();
#endif
}

void MainWindow::on_actionRecent_triggered()
{
    QAction* a = static_cast<QAction*> (QObject::sender());
    QString fileName = a->text();
    if (LoadModel(fileName))
    {
        addToRecentFiles(fileName, false);
    }

}

void MainWindow::removeFromRecentList(QAction* selectedFileAction)
{
    recentFiles.removeAll(selectedFileAction->text());
    ui->menuRecent->removeAction(selectedFileAction);
    writeRecentFilesList();
}

bool MainWindow::CreateFileIfDoesNotExist(QString fileName)
{
    QFileInfo check_file(fileName);
    bool success = false;
    if (!check_file.exists())
    {
        QFile filetobecreated(fileName);
        success = filetobecreated.open(QIODevice::WriteOnly);
        filetobecreated.close();
    }
    return success;
}



