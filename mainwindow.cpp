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
//#include "MCMC.h"


#define version "0.0.7"
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
    connect(ui->treeView,SIGNAL(triggered()),this,SLOT(on_tree_view_triggered()));
    connect(ui->actionTestLevenberg_Marquardt, SIGNAL(triggered()), this, SLOT(on_TestLevenberg_Marquardt()));
    connect(ui->actionConstituent_Properties,SIGNAL(triggered()),this,SLOT(on_constituent_properties_triggered()));
    connect(ui->actionTestDialog,SIGNAL(triggered()),this,SLOT(on_test_dialog_triggered()));
    connect(ui->treeViewtools,SIGNAL(doubleClicked(const QModelIndex&)),this, SLOT(on_tool_executed(const QModelIndex&)));
    connect(ui->TreeView_Results,SIGNAL(doubleClicked(const QModelIndex&)),this, SLOT(on_old_result_requested(const QModelIndex&)));
    connect(ui->actionTestLikelihoods, SIGNAL(triggered()),this,SLOT(on_test_likelihood()));
    connect(ui->actionTestProgressGraph, SIGNAL(triggered()), this, SLOT(on_test_progress_window()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(onAboutTriggered()));
    connect(ui->actionSave_Project,SIGNAL(triggered()),this,SLOT(onSaveProject()));
    connect(ui->actionOpen_Project,SIGNAL(triggered()),this,SLOT(onOpenProject()));
    CGA<SourceSinkData> GA;
    centralform = ui->textBrowser;
    conductor.SetWorkingFolder(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
}

MainWindow::~MainWindow()
{
    if (centralform)
        delete centralform;
    delete ui;
}

void MainWindow::onSaveProject()
{
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save"), "",
            tr("CMB Source file (*.cmb);; All files (*.*)"),nullptr,QFileDialog::DontUseNativeDialog);
    if (!fileName.toLower().contains(".cmb"))
        fileName+=".cmb";
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);


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

    QFile file(fileName);

    if (file.open(QIODevice::ReadOnly))
    {
        Data()->ReadFromFile(&file);
        file.close();
        QFile fileres(fileName);
        fileres.open(QIODevice::ReadOnly);
        QJsonObject jsondoc = QJsonDocument().fromJson(fileres.readAll()).object();
        QJsonObject resultsjson = jsondoc["Results"].toObject();
        resultsviewmodel->clear();
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
    }
}

void MainWindow::on_import_excel()
{
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open"), "",
            tr("Excel files (*.xlsx);; All files (*.*)"),nullptr,QFileDialog::DontUseNativeDialog);

    QFileInfo fi(fileName);
    
    conductor.SetWorkingFolder(fi.absolutePath());
    if (fileName!="")
    {
        ReadExcel(fileName);
    }

    InitiateTables();
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
        for (int j=0; j<y_val.size(); j++)
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
        /* Create the phone groups as QStandardItems */
        QStandardItem *group = new QStandardItem(QString::fromStdString(group_names[i]));
        group->setData("Group",Qt::UserRole);
        /* Append to each group 5 person as children */
        for (map<string,Elemental_Profile>::iterator it= DataCollection.sample_set(group_names[i])->begin();it!=DataCollection.sample_set(group_names[i])->end(); it++)
        {
            QStandardItem *child = new QStandardItem(QString::fromStdString(it->first));
            child->setData("Sample",Qt::UserRole);
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
    for (int elem_counter=0; elem_counter<element_names.size(); elem_counter++)
    {   QStandardItem *element_item=new QStandardItem(QString::fromStdString(element_names[elem_counter]));
        element_item->setData("Element",Qt::UserRole);
        for (int group_counter=0; group_counter<group_names.size(); group_counter++)
        {
            QStandardItem *group = new QStandardItem(QString::fromStdString(group_names[group_counter]));
            group->setData("GroupInElements",Qt::UserRole);
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
        }
        else
        {
            QStandardItem *subitem = new QStandardItem(json.keys()[i]);
            subitem->setData(json.value(json.keys()[i]).toString(),Qt::UserRole);
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
    QAction* showdistributions = menu->addAction("Show fitted distributions");

    QVariant v = QVariant::fromValue(QString::fromStdString("SFD;") + columnviewmodel->itemFromIndex(index)->text());
    showdistributions->setData(v);

    connect(showdistributions, SIGNAL(triggered()), this, SLOT(showdistributionsforelements()));

    if (menu)
        menu->exec( tree->mapToGlobal(pos) );
    return;

}

void MainWindow::showdistributionsforelements()
{
    QAction* act = qobject_cast<QAction*>(sender());
    QStringList keys = act->data().toString().split(";");
    QString key2 = keys[0];
    QString item = keys[1];
    PlotWindow *plotwindow = new PlotWindow(this);
    CTimeSeries<double> elem_dist = DataCollection.FittedDistribution(item.toStdString())->EvaluateAsTimeSeries();
    plotwindow->Plotter()->AddTimeSeries("All samples", elem_dist.tToStdVector(),elem_dist.ValuesToStdVector());
    for (map<string,Elemental_Profile_Set>::iterator it=DataCollection.begin(); it!=DataCollection.end(); it++)
    {
        elem_dist = DataCollection.sample_set(it->first)->ElementalDistribution(item.toStdString())->FittedDistribution()->EvaluateAsTimeSeries();
        plotwindow->Plotter()->AddTimeSeries(it->first, elem_dist.tToStdVector(),elem_dist.ValuesToStdVector());
    }
    plotwindow->setWindowTitle(item);
    plotwindow->Plotter()->SetLegend(true);
    plotwindow->show();

}

void MainWindow::on_constituent_properties_triggered()
{

    if (centralform)
        delete centralform;
    FormElementInformation *formelems = new FormElementInformation(this);
    ElementTableModel *elementtablemodel = new ElementTableModel(&DataCollection,this);
    formelems->table()->setModel(elementtablemodel);
    ElementTableDelegate *elemDelegate = new ElementTableDelegate(&DataCollection, this);
    formelems->table()->setItemDelegate(elemDelegate);
    ui->verticalLayout_middle->addWidget(formelems);
    centralform = formelems;
}

void MainWindow::on_test_dialog_triggered()
{
    if (centralform)
        delete centralform;
    QJsonObject mainjsonobject = formsstructure.object();
    QJsonObject GA_object = mainjsonobject.value("GA").toObject();
    GenericForm *form = new GenericForm(&GA_object,this,this);
    ui->verticalLayout_middle->addWidget(form);
    centralform = form;
}

void MainWindow::on_tool_executed(const QModelIndex &index)
{
    qDebug()<<"Tool executed"<<index.data()<<": "<<index.data(Qt::UserRole);
    if (DataCollection.ElementNames().size()==0)
    {   QMessageBox::warning(this, "OpenHydroQual",tr("No data has been loaded!\n"), QMessageBox::Ok);
        return;
    }
    if (centralform)
    {   delete centralform;
        centralform = nullptr;
    }
    QJsonObject mainjsonobject = formsstructure.object();
    if (mainjsonobject.contains(index.data(Qt::UserRole).toString()))
    {   QJsonObject GA_object = mainjsonobject.value(index.data(Qt::UserRole).toString()).toObject();
        GenericForm *form = new GenericForm(&GA_object,this, this);
        form->SetCommand(index.data(Qt::UserRole).toString());
        ui->verticalLayout_middle->addWidget(form);
        centralform = form;
    }



}

void MainWindow::on_old_result_requested(const QModelIndex& index)
{
    ResultsWindow *reswind = new ResultsWindow();
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
    conductor.SetData(&DataCollection);
    bool outcome = conductor.Execute(command,arguments);
    ResultsWindow *reswind = new ResultsWindow();
    reswind->SetResults(conductor.GetResults());
    ResultSetItem *resultset = new ResultSetItem(QString::fromStdString(conductor.GetResults()->GetName()) + "_" + QDateTime::currentDateTime().toString(Qt::TextDate));
    resultset->setToolTip(QString::fromStdString(conductor.GetResults()->GetName()) + "_" + QDateTime::currentDateTime().toString(Qt::TextDate));
    resultset->result = conductor.GetResults();
    resultsviewmodel->appendRow(resultset);
    for (map<string,ResultItem>::iterator it=resultset->result->begin(); it!=resultset->result->end(); it++)
    {
        reswind->AppendResult(it->second);
    }
    reswind->setWindowTitle(QString::fromStdString(conductor.GetResults()->GetName()) + "_" + QDateTime::currentDateTime().toString(Qt::TextDate));
    reswind->show();

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
