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
    connect(ui->treeView,SIGNAL(triggered()),this,SLOT(on_tree_view_triggered()));
    connect(ui->actionConstituent_Properties,SIGNAL(triggered()),this,SLOT(on_constituent_properties_triggered()));
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
    QList<QStandardItem*> modelitems;
    QList<QStandardItem*> elementitems;


    if (columnviewmodel)
        delete columnviewmodel;
    columnviewmodel = new QStandardItemModel();
    columnviewmodel->setColumnCount(1);
    modelitems.append(ToQStandardItem("Samples",&data));
    columnviewmodel->appendRow(modelitems);
    elementitems.append(ElementsToQStandardItem("Elements",&data));
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
    data.PopulateElementInformation();
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
    map<string,vector<double>> data;

    for (int i=0; i<10; i++)
    {
        vector<double> y_val(i+1);
        for (int j=0; j<y_val.size(); j++)
        {
            y_val[j]=i+0.2*j;
        }
        data["case " + aquiutils::numbertostring(i)] = y_val;
    }
    plotter->AddNoneUniformScatter(data,1);
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

QStandardItem* MainWindow::ToQStandardItem(const QString &key, const SourceSinkData* srcsinkdata)
{

    QStandardItem *columnviewitem = new QStandardItem(key);
    columnviewitem->setData("RootItem",Qt::UserRole);
    vector<string> group_names = data.GroupNames();
    for (int i=0; i<group_names.size(); i++)
    {
        /* Create the phone groups as QStandardItems */
        QStandardItem *group = new QStandardItem(QString::fromStdString(group_names[i]));
        group->setData("Group",Qt::UserRole);
        /* Append to each group 5 person as children */
        for (map<string,Elemental_Profile>::iterator it= data.sample_set(group_names[i])->begin();it!=data.sample_set(group_names[i])->end(); it++)
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
    vector<string> group_names = data.GroupNames();
    vector<string> element_names = data.ElementNames();
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
            vector<string> Sample_Names = data.sample_set(Group_Name_Selected.toStdString())->SampleNames();
            for (int sample_counter=0; sample_counter<Sample_Names.size(); sample_counter++)
            {
                vector<string> item;
                item.push_back(indexes[i].data().toString().toStdString());
                item.push_back(indexes[i].child(sample_counter,0).data().toString().toStdString());
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
            map<string,vector<double>> extracted_data = data.ExtractElementData(Element_Name_Selected.toStdString());
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
    {   profiles_data extracted_data = data.ExtractData(samples_selected);
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
    CTimeSeries<double> elem_dist = data.FittedDistribution(item.toStdString())->EvaluateAsTimeSeries();
    plotwindow->Plotter()->AddTimeSeries("All samples", elem_dist.tToStdVector(),elem_dist.ValuesToStdVector());
    for (map<string,Elemental_Profile_Set>::iterator it=data.begin(); it!=data.end(); it++)
    {
        elem_dist = data.sample_set(it->first)->ElementalDistribution(item.toStdString())->FittedDistribution()->EvaluateAsTimeSeries();
        plotwindow->Plotter()->AddTimeSeries(it->first, elem_dist.tToStdVector(),elem_dist.ValuesToStdVector());
    }
    plotwindow->setWindowTitle(item);
    plotwindow->Plotter()->SetLegend(true);
    plotwindow->show();

}

void MainWindow::on_constituent_properties_triggered()
{
    ui->textBrowser->hide();
    FormElementInformation *formelems = new FormElementInformation(this);
    ElementTableModel *elementtablemodel = new ElementTableModel(&data,this);
    formelems->table()->setModel(elementtablemodel);
    ElementTableDelegate *elemDelegate = new ElementTableDelegate(&data, this);
    formelems->table()->setItemDelegate(elemDelegate);
    ui->verticalLayout_middle->addWidget(formelems);
}
