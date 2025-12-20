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
#include "ProgressWindow.h"
#include "resultswindow.h"
#include "aboutdialog.h"
#include "resultsetitem.h"
#include "resultitem.h"
#include "selectsamples.h"
#include "dialogchooseexcelsheets.h"
#include "toolboxitem.h"
#include "resulttableviewer.h"
#include "optionsdialog.h"
//#include "MCMC.h"

#ifdef _WIN32
#define NOBYTE
//#include <windows.h>

//#pragma comment(lib "shell32.lib")
#endif

#define RECENT "SedSatrecentFiles.txt"

#ifndef max_num_recent_files
#define max_num_recent_files 15
#endif


#define version "1.1.3"
#define date_compiled "12/20/2025"

using namespace QXlsx;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    sinkSheet(-1),
    plotter(nullptr),
    columnViewModel(nullptr),
    resultsViewModel(nullptr),
    ResultscontextMenu(nullptr),
    DeleteAction(nullptr),
    selectedTreeItemType("None"),
    treeItemChangedProgramatically(false),
    conductor(nullptr)
{
    ui->setupUi(this);

    // Set window icon
    QIcon mainIcon(qApp->applicationDirPath() + "/../../resources/Icons/CMBSource_Icon.png");
    setWindowIcon(mainIcon);

    // Initialize conductor
    conductor = std::make_unique<Conductor>(this);
    conductor->SetWorkingFolder(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));

    // Setup UI components
    setupToolsView();
    setupResultsView();
    setupToolBar();
    setupConnections();

    // Initialize other components
    centralform.reset(ui->textBrowser);

    // Load recent files
    readRecentFilesList();
}

void MainWindow::setupConnections()
{
    // Menu actions - File operations
    connect(ui->actionImport_Elemental_Profile_from_Excel, &QAction::triggered,
        this, &MainWindow::on_import_excel);
    connect(ui->actionSave_Project, &QAction::triggered,
        this, &MainWindow::onSaveProject);
    connect(ui->actionSave_As, &QAction::triggered,
        this, &MainWindow::onSaveProjectAs);
    connect(ui->actionOpen_Project, &QAction::triggered,
        this, &MainWindow::onOpenProject);

    // Menu actions - Plotting
    connect(ui->actionRaw_Elemental_Profiles, &QAction::triggered,
        this, &MainWindow::on_plot_raw_elemental_profiles);

    // Menu actions - Data operations
    connect(ui->actionConstituent_Properties, &QAction::triggered,
        this, &MainWindow::on_constituent_properties_triggered);
    connect(ui->actionInclude_Exclude_Samples, &QAction::triggered,
        this, &MainWindow::onIncludeExcludeSample);
    connect(ui->actionOrganic_Matter_Size_correction, &QAction::triggered,
        this, &MainWindow::onOMSizeCorrection);

    // Menu actions - Help and settings
    connect(ui->actionAbout, &QAction::triggered,
        this, &MainWindow::onAboutTriggered);
    connect(ui->actionOptions, &QAction::triggered,
        this, &MainWindow::on_Options_triggered);

    // Toolbar buttons
    connect(ui->ShowTabular, &QPushButton::clicked,
        this, &MainWindow::on_show_data_as_table);
    connect(ui->btnZoom, &QPushButton::clicked,
        this, &MainWindow::on_ZoomExtends);

    // Tree views
    connect(ui->treeViewtools, &QTreeView::doubleClicked,
        this, &MainWindow::on_tool_executed);
    connect(ui->TreeView_Results, &QTreeView::doubleClicked,
        this, &MainWindow::on_old_result_requested);

    // Results context menu
    connect(ui->TreeView_Results, &QWidget::customContextMenuRequested,
        this, &MainWindow::onCustomContextMenu);
    connect(DeleteAction, &QAction::triggered,
        this, &MainWindow::DeleteResults);
}

void MainWindow::setupToolsView()
{
    // Load tools configuration from JSON
    QString toolsPath = qApp->applicationDirPath() + "/../../resources/tools.json";
    qDebug() << "Loading tools from:" << toolsPath;

    QJsonDocument tools = loadJson(toolsPath);
    QStandardItemModel* toolsModel = ToQStandardItemModel(tools);
    toolsModel->setHorizontalHeaderLabels(QStringList() << "Tools");

    ui->treeViewtools->setModel(toolsModel);
    ui->treeViewtools->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Load forms structure
    QString formsPath = qApp->applicationDirPath() + "/../../resources/forms_structures.json";
    formsstructure = loadJson(formsPath);

    // Configure tree view settings
    ui->treeView->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->frame->setVisible(false);
}

void MainWindow::setupResultsView()
{
    // Initialize results model
    resultsViewModel.reset(new QStandardItemModel());
    resultsViewModel->setHorizontalHeaderLabels(QStringList() << "Results");

    ui->TreeView_Results->setModel(resultsViewModel.get());
    ui->TreeView_Results->setContextMenuPolicy(Qt::CustomContextMenu);

    // Setup context menu
    ResultscontextMenu = new QMenu(ui->TreeView_Results);
    DeleteAction = new QAction("Delete", ResultscontextMenu);
    ResultscontextMenu->addAction(DeleteAction);
}

void MainWindow::setupToolBar()
{
    // Setup table view button
    QIcon iconTable(qApp->applicationDirPath() + "/../../resources/Icons/table.png");
    ui->ShowTabular->setIcon(iconTable);
    ui->ShowTabular->setEnabled(false);

    // Setup zoom button
    QIcon iconZoom(qApp->applicationDirPath() + "/../../resources/Icons/Zoom_Extends.png");
    ui->btnZoom->setIcon(iconZoom);
    ui->btnZoom->setEnabled(false);

    // Call existing toolbar population method
    Populate_General_ToolBar();
}

MainWindow::~MainWindow()
{
    // plotter is added to layout, so Qt handles it
    // ui is deleted explicitly as always
    delete ui;
}

QString MainWindow::getSaveFilePath(bool promptUser)
{
    // Use existing project file name if available and not prompting
    if (!promptUser && !ProjectFileName.isEmpty())
    {
        return ProjectFileName;
    }

    // Show save file dialog
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save Project"),
        ProjectFileName,  // Simplified - empty QString is fine
        tr("CMB Source file (*.cmb);;All files (*.*)"),
        nullptr,
        QFileDialog::DontUseNativeDialog
    );

    // User cancelled
    if (fileName.isEmpty())
    {
        return QString();
    }

    // Ensure .cmb extension
    if (!fileName.toLower().endsWith(".cmb"))
    {
        fileName += ".cmb";
    }

    return fileName;
}

QJsonObject MainWindow::buildResultsJson() const
{
    QJsonObject resultsJson;

    if (!resultsViewModel)
    {
        return resultsJson;
    }

    for (int i = 0; i < resultsViewModel->rowCount(); ++i)
    {
        QStandardItem* item = resultsViewModel->item(i, 0);
        if (!item)
        {
            continue;
        }

        ResultSetItem* resultSetItem = dynamic_cast<ResultSetItem*>(item);
        if (!resultSetItem || !resultSetItem->result)
        {
            qWarning() << "Invalid result item at row" << i;
            continue;
        }

        resultsJson[resultSetItem->text()] = resultSetItem->result->toJsonObject();
    }

    return resultsJson;
}

QJsonObject MainWindow::buildProjectJson() const
{
    QJsonObject projectJson;

    projectJson["Element Information"] = Data()->ElementInformationToJsonObject();
    projectJson["Element Data"] = Data()->ElementDataToJsonObject();
    projectJson["Target Group"] = QString::fromStdString(Data()->GetTargetGroup());
    projectJson["Tools used"] = Data()->ToolsUsedToJsonObject();
    projectJson["Options"] = Data()->OptionsToJsonObject();
    projectJson["Results"] = buildResultsJson();

    return projectJson;
}

bool MainWindow::saveProjectToFile(const QString& filePath)
{
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(
            this,
            tr("Save Error"),
            tr("Could not open file for writing:\n%1\n\nError: %2")
            .arg(filePath)
            .arg(file.errorString())
        );
        return false;
    }

    // Set working folder based on file location
    QFileInfo fileInfo(file);
    conductor->SetWorkingFolder(fileInfo.absolutePath());

    // Build and write JSON
    QJsonObject projectJson = buildProjectJson();
    QJsonDocument jsonDocument(projectJson);

    qint64 bytesWritten = file.write(jsonDocument.toJson());
    file.close();

    if (bytesWritten == -1)
    {
        QMessageBox::critical(
            this,
            tr("Save Error"),
            tr("Error writing to file:\n%1\n\nError: %2")
            .arg(filePath)
            .arg(file.errorString())
        );
        return false;
    }

    // Update project state
    ProjectFileName = filePath;
    addToRecentFiles(filePath, true);

    return true;
}

void MainWindow::onSaveProject()
{
    QString filePath = getSaveFilePath(false);

    if (filePath.isEmpty())
    {
        return; // User cancelled
    }

    if (saveProjectToFile(filePath))
    {
        statusBar()->showMessage(tr("Project saved successfully"), 3000);
    }
}

void MainWindow::onSaveProjectAs()
{
    QString filePath = getSaveFilePath(true);

    if (filePath.isEmpty())
    {
        return; // User cancelled
    }

    if (saveProjectToFile(filePath))
    {
        statusBar()->showMessage(tr("Project saved successfully"), 3000);
    }
}

void MainWindow::onOpenProject()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open Project"),
        ProjectFileName,  // Start in same directory as current project
        tr("CMB Source file (*.cmb);;All files (*.*)"),
        nullptr,
        QFileDialog::DontUseNativeDialog
    );

    if (fileName.isEmpty())
    {
        return;
    }

    if (LoadModel(fileName))
    {
        addToRecentFiles(fileName, true);
        statusBar()->showMessage(tr("Project opened successfully"), 3000);
    }
}

bool MainWindow::LoadModel(const QString& fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(
            this,
            tr("Load Error"),
            tr("Could not open file for reading:\n%1\n\nError: %2")
            .arg(fileName)
            .arg(file.errorString())
        );
        return false;
    }

    // Read and parse JSON
    QByteArray fileData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);
    if (jsonDoc.isNull())
    {
        QMessageBox::critical(
            this,
            tr("Load Error"),
            tr("Invalid JSON format in file:\n%1")
            .arg(fileName)
        );
        return false;
    }

    QJsonObject jsonObject = jsonDoc.object();

    // Set working directory
    QFileInfo fileInfo(fileName);
    conductor->SetWorkingFolder(fileInfo.absolutePath());

    // Load data
    file.open(QIODevice::ReadOnly);
    Data()->ReadFromFile(&file);
    file.close();

    // Load results
    QJsonObject resultsJson = jsonObject["Results"].toObject();
    resultsViewModel->clear();
    resultsViewModel->setHorizontalHeaderLabels(QStringList() << "Results");

    for (const QString& key : resultsJson.keys())
    {
        ResultSetItem* resultSet = new ResultSetItem(key);
        resultSet->setToolTip(key);
        resultSet->result = new Results();
        resultSet->result->ReadFromJson(resultsJson.value(key).toObject());

        // Process MLR results
        for (auto it = resultSet->result->begin(); it != resultSet->result->end(); ++it)
        {
            if (it->second.Type() == result_type::mlrset)
            {
                QStringList parts = QString::fromStdString(it->first).split("OM & Size MLR for ");
                if (parts.size() > 1)
                {
                    std::string source = parts[1].toStdString();
                    if (Data()->count(source) > 0)
                    {
                        Data()->at(source).SetRegressionModels(
                            static_cast<MultipleLinearRegressionSet*>(it->second.Result())
                        );
                        Data()->SetOMandSizeConstituents(
                            Data()->at(source).GetRegressionModels()->begin()->second.GetIndependentVariableNames()
                        );
                    }
                }
            }
        }

        resultsViewModel->appendRow(resultSet);
    }

    // Finalize data
    dataCollection.PopulateElementDistributions();
    dataCollection.AssignAllDistributions();
    InitiateTables();

    // Update UI state
    ProjectFileName = fileName;
    setWindowTitle("SedSat3: " + ProjectFileName);

    return true;
}

void MainWindow::on_import_excel()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open Excel File"),
        ProjectFileName.isEmpty() ? "" : QFileInfo(ProjectFileName).absolutePath(),
        tr("Excel files (*.xlsx);;All files (*.*)"),
        nullptr,
        QFileDialog::DontUseNativeDialog
    );

    if (fileName.isEmpty())
    {
        return; // User cancelled
    }

    QFileInfo fileInfo(fileName);
    conductor->SetWorkingFolder(fileInfo.absolutePath());

    if (ReadExcel(fileName))
    {
        InitiateTables();

        QMessageBox::information(
            this,
            tr("Element Selection"),
            tr("To exclude elements from analysis, double-click on an element and uncheck the box."),
            QMessageBox::Ok
        );

        on_constituent_properties_triggered();
    }
}
void MainWindow::InitiateTables()
{
    // Create new model
    columnViewModel = std::make_unique<QStandardItemModel>();
    columnViewModel->setColumnCount(1);
    columnViewModel->setHorizontalHeaderLabels(QStringList() << "Data");

    // Add samples item
    QList<QStandardItem*> modelItems;
    modelItems.append(ToQStandardItem("Samples", &dataCollection));
    columnViewModel->appendRow(modelItems);

    // Add elements item
    QList<QStandardItem*> elementItems;
    elementItems.append(ElementsToQStandardItem("Elements", &dataCollection));
    columnViewModel->appendRow(elementItems);

    // Set model on tree view
    ui->treeView->setModel(columnViewModel.get());
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    // Connect signals - modern Qt5 syntax
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &MainWindow::on_tree_selectionChanged);
    connect(ui->treeView, &QWidget::customContextMenuRequested,
        this, &MainWindow::preparetreeviewMenu);
}
void MainWindow::on_plot_raw_elemental_profiles()
{
    PlotWindow *plotwindow = new PlotWindow(this);
    plotwindow->show();
}

bool MainWindow::ReadExcel(const QString& filename)
{
    // Load Excel file
    Document xlsxR(filename);
    if (!xlsxR.load())
    {
        QMessageBox::critical(
            this,
            tr("Excel Load Error"),
            tr("Failed to load Excel file:\n%1").arg(filename)
        );
        return false;
    }

    qDebug() << "[debug] Successfully loaded xlsx file.";
    QStringList allSheetNames = xlsxR.sheetNames();

    // Clear existing data
    dataCollection.Clear();

    // Let user select sheets to import
    QStringList* sheetsToInclude = nullptr;
    auto chooseSheetsDlg = std::make_unique<DialogChooseExcelSheets>(this, &sheetsToInclude);

    for (const QString& sheetName : allSheetNames)
    {
        chooseSheetsDlg->AddItem(sheetName);
        qDebug() << sheetName;
    }

    chooseSheetsDlg->exec();

    if (!sheetsToInclude)
    {
        return false; // User cancelled
    }

    // Filter selected sheets
    QStringList selectedSheets;
    for (const QString& sheetName : allSheetNames)
    {
        if (sheetsToInclude->contains(sheetName))
        {
            selectedSheets << sheetName;
        }
    }
    delete sheetsToInclude;

    if (selectedSheets.isEmpty())
    {
        return false; // No sheets selected
    }

    // Let user indicate which sheet is the sink
    auto indicateSheetDlg = std::make_unique<IndicateSheetsDialog>(this);
    indicateSheetDlg->Populate_Table(selectedSheets);
    indicateSheetDlg->exec();

    qDebug() << "Sink Sheet is:" << sinkSheet;

    // Validate element names are consistent across all sheets
    QList<QStringList> elementNamesBySheet;

    for (int sheetIdx = 0; sheetIdx < selectedSheets.count(); ++sheetIdx)
    {
        xlsxR.selectSheet(selectedSheets[sheetIdx]);
        QStringList elementNames;

        // Read element names from row 1, starting at column 2
        int col = 2;
        while (true)
        {
            auto cell = xlsxR.cellAt(1, col);  // Returns shared_ptr
            if (!cell)
                break;

            QString elemName = cell->readValue().toString().trimmed();
            if (elemName.isEmpty())
                break;

            elementNames.append(elemName);
            qDebug() << elemName;
            col++;
        }

        // Verify element names match across sheets
        if (sheetIdx > 0 && elementNames != elementNamesBySheet.last())
        {
            QMessageBox::warning(
                this,
                tr("Element Mismatch"),
                tr("Element names and their order must be identical in all sheets.\n\n"
                    "Mismatch found between:\n  '%1'\n  '%2'")
                .arg(selectedSheets[sheetIdx - 1])
                .arg(selectedSheets[sheetIdx])
            );

            WriteMessageOnScreen(
                QString("Element mismatch between sheets '%1' and '%2'")
                .arg(selectedSheets[sheetIdx - 1])
                .arg(selectedSheets[sheetIdx]),
                Qt::red
            );
            return false;
        }

        elementNamesBySheet.append(elementNames);
    }

    // Read sample data from each sheet
    for (int sheetIdx = 0; sheetIdx < selectedSheets.count(); ++sheetIdx)
    {
        xlsxR.selectSheet(selectedSheets[sheetIdx]);

        Elemental_Profile_Set* profileSet = dataCollection.AppendSampleSet(
            selectedSheets[sheetIdx].toStdString()
        );

        // Read samples starting from row 2
        int row = 2;
        while (true)
        {
            auto sampleCell = xlsxR.cellAt(row, 1);
            if (!sampleCell || sampleCell->readValue().toString().isEmpty())
                break;

            QString sampleName = sampleCell->readValue().toString();
            Elemental_Profile elementalProfile;

            // Read element values for this sample
            for (int col = 0; col < elementNamesBySheet[0].count(); ++col)
            {
                auto dataCell = xlsxR.cellAt(row, col + 2);

                if (!dataCell)
                {
                    QMessageBox::warning(
                        this,
                        tr("Empty Cell"),
                        tr("Empty cell found in:\n"
                            "Sheet: %1\n"
                            "Row: %2\n"
                            "Column: %3")
                        .arg(selectedSheets[sheetIdx])
                        .arg(row)
                        .arg(col + 2)
                    );
                    dataCollection.Clear();
                    return false;
                }

                bool isNumber = false;
                double value = dataCell->readValue().toDouble(&isNumber);

                if (!isNumber)
                {
                    QMessageBox::warning(
                        this,
                        tr("Invalid Data"),
                        tr("Non-numerical value found:\n"
                            "Sheet: %1\n"
                            "Row: %2\n"
                            "Column: %3\n"
                            "Value: '%4'")
                        .arg(selectedSheets[sheetIdx])
                        .arg(row)
                        .arg(col + 2)
                        .arg(dataCell->readValue().toString())
                    );
                    dataCollection.Clear();
                    return false;
                }

                elementalProfile.AppendElement(
                    elementNamesBySheet[0][col].toStdString(),
                    value
                );
            }

            profileSet->AppendProfile(sampleName.toStdString(), elementalProfile);
            row++;
        }
    }

    // Finalize data collection
    dataCollection.PopulateElementInformation();
    dataCollection.PopulateElementDistributions();
    dataCollection.AssignAllDistributions();

    qDebug() << "Reading element information done!";

    return true;
}

void MainWindow::WriteMessageOnScreen(const QString &text, QColor color)
{
    ui->textBrowser->setTextColor(color);
    ui->textBrowser->append(text);
}

QStandardItem* MainWindow::ToQStandardItem(const QString& key, const SourceSinkData* srcsinkdata)
{
    QStandardItem* rootItem = new QStandardItem(key);
    rootItem->setData("RootItem", Qt::UserRole);

    vector<string> groupNames = dataCollection.GetGroupNames();

    for (size_t i = 0; i < groupNames.size(); ++i)
    {
        QStandardItem* groupItem = new QStandardItem(QString::fromStdString(groupNames[i]));
        groupItem->setData("Group", Qt::UserRole);
        groupItem->setData(QString::fromStdString(groupNames[i]), groupRole);

        Elemental_Profile_Set* sampleSet = dataCollection.GetSampleSet(groupNames[i]);
        if (sampleSet)
        {
            for (auto it = sampleSet->begin(); it != sampleSet->end(); ++it)
            {
                QStandardItem* sampleItem = new QStandardItem(QString::fromStdString(it->first));
                sampleItem->setData("Sample", Qt::UserRole);
                sampleItem->setData(QString::fromStdString(groupNames[i]), groupRole);
                sampleItem->setData(QString::fromStdString(it->first), sampleRole);

                groupItem->appendRow(sampleItem);
            }
        }

        rootItem->appendRow(groupItem);
    }

    return rootItem;
}

QStandardItem* MainWindow::ElementsToQStandardItem(const QString& key, const SourceSinkData* srcsinkdata)
{
    QStandardItem* rootItem = new QStandardItem(key);
    rootItem->setData("RootItem", Qt::UserRole);

    vector<string> groupNames = dataCollection.GetGroupNames();
    vector<string> elementNames = dataCollection.GetElementNames();

    for (size_t elemIdx = 0; elemIdx < elementNames.size(); ++elemIdx)
    {
        QStandardItem* elementItem = new QStandardItem(QString::fromStdString(elementNames[elemIdx]));
        elementItem->setData("Element", Qt::UserRole);
        elementItem->setData(QString::fromStdString(elementNames[elemIdx]), elementRole);

        for (size_t groupIdx = 0; groupIdx < groupNames.size(); ++groupIdx)
        {
            QStandardItem* groupItem = new QStandardItem(QString::fromStdString(groupNames[groupIdx]));
            groupItem->setData("GroupInElements", Qt::UserRole);
            groupItem->setData(QString::fromStdString(elementNames[elemIdx]), elementRole);
            groupItem->setData(QString::fromStdString(groupNames[groupIdx]), groupRole);

            elementItem->appendRow(groupItem);
        }

        rootItem->appendRow(elementItem);
    }

    return rootItem;
}

QStandardItemModel* MainWindow::ToQStandardItemModel(const SourceSinkData* srcsinkdata)
{
    columnViewModel = std::make_unique<QStandardItemModel>();
    columnViewModel->setColumnCount(1);
    columnViewModel->setHorizontalHeaderLabels(QStringList() << "Groups");

    vector<string> groupNames = dataCollection.GetGroupNames();

    for (size_t i = 0; i < groupNames.size(); ++i)
    {
        QStandardItem* groupItem = new QStandardItem(QString::fromStdString(groupNames[i]));
        groupItem->setData("Parent", Qt::UserRole);

        Elemental_Profile_Set* sampleSet = dataCollection.GetSampleSet(groupNames[i]);
        if (sampleSet)
        {
            for (auto it = sampleSet->begin(); it != sampleSet->end(); ++it)
            {
                QStandardItem* sampleItem = new QStandardItem(QString::fromStdString(it->first));
                sampleItem->setData(QString::fromStdString(it->first), elementRole);
                sampleItem->setData(QString::fromStdString(groupNames[i]), groupRole);
                sampleItem->setData("Child", Qt::UserRole);

                groupItem->appendRow(sampleItem);
            }
        }

        columnViewModel->appendRow(groupItem);
    }

    return columnViewModel.get();
}
void MainWindow::on_tree_selectionChanged(const QItemSelection &changed)
{
    qDebug()<<"Selection changed "<<changed;
    ui->ShowTabular->setEnabled(false);
    if (treeItemChangedProgramatically) return;
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
                if (changed.at(0).indexes()[0].data(Qt::UserRole)!=selectedTreeItemType && selectedTreeItemType!="None")
                {
                    selectedTreeItemType = changed.at(0).indexes()[0].data(Qt::UserRole).toString();
                    treeItemChangedProgramatically=true;
                    ui->treeView->selectionModel()->clearSelection();
                    ui->treeView->selectionModel()->select(changed_index,QItemSelectionModel::Select | QItemSelectionModel::Rows);
                    treeItemChangedProgramatically=false;
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
            vector<string> Sample_Names = dataCollection.GetSampleSet(Group_Name_Selected.toStdString())->GetSampleNames();
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
            map<string,vector<double>> extracted_data = dataCollection.ExtractElementDataByGroup(Element_Name_Selected.toStdString());
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
    {   profiles_data extracted_data = dataCollection.ExtractConcentrationData(samples_selected);
    plottedData = std::make_unique<Elemental_Profile_Set>(
        dataCollection.ExtractSamplesAsProfileSet(samples_selected)  
    );
        ui->ShowTabular->setEnabled(true);
        plotter->AddScatters(extracted_data.sample_names,extracted_data.element_names, extracted_data.values);
        plotter->SetYAxisScaleType(AxisScale::log);
    }
    ui->frame->setVisible(true);
    ui->frame->setEnabled(true);
    ui->btnZoom->setEnabled(true);
    selectedTreeItemType = TreeQStringSelectedType();
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

QStandardItem* MainWindow::ToQStandardItem(const QString& key, const QJsonObject& json)
{
    QStandardItem* standardItem = new QStandardItem(key);

    QStringList keys = json.keys();
    for (const QString& jsonKey : keys)
    {
        if (json[jsonKey].isObject())
        {
            // Recursively create items for nested objects
            standardItem->appendRow(ToQStandardItem(jsonKey, json[jsonKey].toObject()));
            standardItem->setToolTip(jsonKey);
        }
        else
        {
            // Create leaf items for values
            QStandardItem* subItem = new ToolBoxItem(Data(), jsonKey);
            subItem->setData(json.value(jsonKey).toString(), Qt::UserRole);
            subItem->setData(json.value(jsonKey).toString(), Qt::UserRole + 1);
            subItem->setToolTip(jsonKey);
            standardItem->appendRow(subItem);
        }
    }

    return standardItem;
}

QStandardItemModel* MainWindow::ToQStandardItemModel(const QJsonDocument& jsonDocument)
{
    QStandardItemModel* standardItemModel = new QStandardItemModel();
    QJsonObject jsonObject = jsonDocument.object();
    QStandardItem* rootItem = ToQStandardItem("Tools", jsonObject);

    standardItemModel->appendRow(rootItem);

    return standardItemModel;
}

QString MainWindow::TreeQStringSelectedType()
{
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();

    if (!selectedIndexes.isEmpty())
    {
        return selectedIndexes[0].data(Qt::UserRole).toString();
    }

    return "none";
}

void MainWindow::preparetreeviewMenu(const QPoint& pos)
{
    QTreeView* tree = ui->treeView;
    QModelIndex index = tree->currentIndex();

    if (!index.isValid())
    {
        return;
    }

    qDebug() << index.data(elementRole);
    qDebug() << index.data(groupRole);
    qDebug() << index.data(sampleRole);

    menu = std::make_unique<QMenu>(this);

    if (!index.data(elementRole).toString().isEmpty())
    {
        QAction* showDistributions = menu->addAction("Show fitted distributions");

        QStringList roleData;
        roleData << "Element=" + index.data(elementRole).toString();
        roleData << "Group=" + index.data(groupRole).toString();
        roleData << "Sample=" + index.data(sampleRole).toString();
        roleData << "Action=SFD";

        showDistributions->setData(roleData);

        connect(showDistributions, &QAction::triggered,
            this, &MainWindow::showdistributionsforelements);
    }

    if (menu)
    {
        menu->exec(tree->mapToGlobal(pos));
    }
}

void MainWindow::showdistributionsforelements()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action)
    {
        return;
    }

    QStringList keysStringList = action->data().toStringList();
    QMap<QString, QString> keys;

    for (const QString& keyValue : keysStringList)
    {
        QStringList parts = keyValue.split("=");
        if (parts.size() == 2)
        {
            keys[parts[0]] = parts[1];
        }
    }

    QString element = keys["Element"];
    QString group = keys["Group"];
    QString sample = keys["Sample"];
    QString actionType = keys["Action"];

    if (actionType != "SFD")
    {
        return;
    }

    PlotWindow* plotWindow = new PlotWindow(this);

    if (group.isEmpty())
    {
        // Plot distribution for all samples
        TimeSeries<double> elementDist =
            dataCollection.GetFittedDistribution(element.toStdString())->EvaluateAsTimeSeries();

        plotWindow->Plotter()->AddTimeSeries(
            "All samples",
            elementDist.tToStdVector(),
            elementDist.ValuesToStdVector()
        );

        // Add distribution for each group
        for (auto it = dataCollection.begin(); it != dataCollection.end(); ++it)
        {
            elementDist = dataCollection.GetSampleSet(it->first)
                ->GetElementDistribution(element.toStdString())
                ->GetFittedDistribution()
                ->EvaluateAsTimeSeries();

            plotWindow->Plotter()->AddTimeSeries(
                it->first,
                elementDist.tToStdVector(),
                elementDist.ValuesToStdVector()
            );
        }
    }
    else
    {
        // Plot distribution for specific group
        TimeSeries<double> elementDist =
            dataCollection.GetSampleSet(group.toStdString())
            ->GetElementDistribution(element.toStdString())
            ->GetFittedDistribution()
            ->EvaluateAsTimeSeries();

        plotWindow->Plotter()->AddTimeSeries(
            (group + ":" + element).toStdString(),
            elementDist.tToStdVector(),
            elementDist.ValuesToStdVector()
        );
    }

    plotWindow->setWindowTitle(element);
    plotWindow->Plotter()->SetLegend(true);
    plotWindow->show();
}

void MainWindow::on_constituent_properties_triggered()
{
    FormElementInformation* formElems = new FormElementInformation(this);
    ElementTableModel* elementTableModel = new ElementTableModel(&dataCollection, this);
    formElems->table()->setModel(elementTableModel);

    ElementTableDelegate* elemDelegate = new ElementTableDelegate(&dataCollection, this);
    formElems->table()->setItemDelegate(elemDelegate);

    ui->verticalLayout_middle->addWidget(formElems);
    centralform.reset(formElems);
}

void MainWindow::onIncludeExcludeSample()
{
    double outlierThreshold = Data()->GetOptions()->operator[]("Outlier deviation threshold");

    // Prompt for options if using default threshold
    if (outlierThreshold == 3)
    {
        on_Options_triggered();
    }

    // Perform outlier analysis
    Data()->OutlierAnalysisForAll(-outlierThreshold, outlierThreshold);
    Data()->BracketTest(false, false, false);

    // Create and configure sample selection dialog
    SelectSamples* sampleSelector = new SelectSamples(this);
    sampleSelector->SetMode(mode::samples);
    sampleSelector->SetData(&dataCollection);

    ui->verticalLayout_middle->addWidget(sampleSelector);
    centralform.reset(sampleSelector);
}

void MainWindow::onOMSizeCorrection()
{
    // Check if OM and size correction has been performed
    if (dataCollection.OMandSizeConstituents()[0].empty() &&
        dataCollection.OMandSizeConstituents()[1].empty())
    {
        QMessageBox::warning(
            this,
            tr("SedSat3"),
            tr("Organic Matter and Size Correction must be performed first."),
            QMessageBox::Ok
        );
        return;
    }

    // Create and configure sample selection dialog
    SelectSamples* sampleSelector = new SelectSamples(this);
    sampleSelector->SetMode(mode::regressions);
    sampleSelector->SetData(&dataCollection);

    ui->verticalLayout_middle->addWidget(sampleSelector);
    centralform.reset(sampleSelector);
}


void MainWindow::on_tool_executed(const QModelIndex& index)
{
    qDebug() << "Tool executed" << index.data() << ":" << index.data(Qt::UserRole);

    // Check if data has been loaded
    if (dataCollection.GetElementNames().empty())
    {
        QMessageBox::warning(
            this,
            tr("SedSat3"),
            tr("No data has been loaded."),
            QMessageBox::Ok
        );
        return;
    }

    QJsonObject mainJsonObject = formsstructure.object();
    QString toolName = index.data(Qt::UserRole).toString();

    if (mainJsonObject.contains(toolName))
    {
        qDebug() << toolName;

        QJsonObject formObject = mainJsonObject.value(toolName).toObject();

        centralform = std::make_unique<GenericForm>(&formObject, this, this);
        dynamic_cast<GenericForm*>(centralform.get())->SetCommand(toolName);
        ui->verticalLayout_middle->addWidget(centralform.get());
    }
}

void MainWindow::on_old_result_requested(const QModelIndex& index)
{
    if (!index.isValid() || !resultsViewModel)
    {
        return;
    }

    QStandardItem* item = resultsViewModel->item(index.row());
    if (!item)
    {
        return;
    }

    ResultSetItem* resultSetItem = dynamic_cast<ResultSetItem*>(item);
    if (!resultSetItem || !resultSetItem->result)
    {
        qWarning() << "Invalid result item at row" << index.row();
        return;
    }

    Results* resultSet = resultSetItem->result;

    ResultsWindow* resultsWindow = new ResultsWindow(this);
    resultsWindow->SetResults(resultSet);
    resultsWindow->setWindowTitle(resultSetItem->text());

    for (auto it = resultSet->begin(); it != resultSet->end(); ++it)
    {
        resultsWindow->AppendResult(it->second);
    }

    resultsWindow->show();
}

bool MainWindow::Execute(const std::string& command, std::map<std::string, std::string> arguments)
{
    conductor->SetData(&dataCollection);

    bool outcome = conductor->Execute(command, arguments);
    if (!outcome)
    {
        return false;
    }

    // Create separate copies for window and permanent storage
    Results* resultsForWindow = conductor->GetResults();   // For ResultsWindow
    Results* resultsForStorage = conductor->GetResults();  // For ResultSetItem (persists)

    // Generate timestamp suffix
    QString timestamp = QDateTime::currentDateTime().toString(Qt::TextDate);
    QString resultName = QString::fromStdString(resultsForStorage->GetName()) + "_" + timestamp;

    // Create result set item for permanent storage
    ResultSetItem* resultSetItem = new ResultSetItem(resultName);
    resultSetItem->setToolTip(resultName);
    resultSetItem->result = resultsForStorage;
    resultsViewModel->appendRow(resultSetItem);

    // Create results window
    ResultsWindow* resultsWindow = new ResultsWindow(this);
    resultsWindow->SetResults(resultsForWindow);
    resultsWindow->setWindowTitle(resultName);

    // Populate results window
    for (auto it = resultsForStorage->begin(); it != resultsForStorage->end(); ++it)
    {
        resultsWindow->AppendResult(it->second);
    }

    resultsWindow->show();

    return true;
}

void MainWindow::onAboutTriggered()
{
    AboutDialog* aboutDlg = new AboutDialog(this);
    aboutDlg->setVersion(version);
    aboutDlg->setBuildDate(date_compiled);
    aboutDlg->exec();
    delete aboutDlg;
}

void MainWindow::onCustomContextMenu(const QPoint& point)
{
    indexresultselected = ui->TreeView_Results->indexAt(point);
    if (indexresultselected.isValid()) {
        ResultscontextMenu->exec(ui->TreeView_Results->viewport()->mapToGlobal(point));
    }
}

void MainWindow::DeleteResults()
{
    if (!indexresultselected.isValid())
    {
        qWarning() << "Attempted to delete with invalid index";
        return;
    }

    QStandardItem* item = resultsViewModel->item(indexresultselected.row());
    if (item)
    {
        int ret = QMessageBox::question(
            this,
            tr("Delete Result"),
            tr("Are you sure you want to delete '%1'?").arg(item->text()),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );

        if (ret == QMessageBox::Yes)
        {
            resultsViewModel->removeRow(indexresultselected.row());
        }
    }
}

void MainWindow::readRecentFilesList()
{
    QString recentFilePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
        + "/" + RECENT;

    QFile file(recentFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "No recent files list found";
        return;
    }

    QTextStream in(&file);
    QStringList allLines;

    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (!line.isEmpty())
        {
            allLines.append(line);
        }
    }
    file.close();

    // Add only the most recent files (up to max_num_recent_files)
    int startIndex = qMax(0, allLines.size() - max_num_recent_files);
    for (int i = startIndex; i < allLines.size(); ++i)
    {
        addToRecentFiles(allLines[i], false);
    }
}

void MainWindow::addToRecentFiles(const QString& fileName, bool addToFile)
{
    if (fileName.trimmed().isEmpty())
    {
        return;
    }

    QString trimmedFileName = fileName.trimmed();
    bool rewriteFile = false;

    // If file already exists in list, move it to the end (most recent)
    if (recentFiles.contains(trimmedFileName))
    {
        int existingIndex = recentFiles.indexOf(trimmedFileName);

        // Only move if not already at the end
        if (existingIndex != recentFiles.count() - 1)
        {
            // Remove from menu
            QList<QAction*> actions = ui->menuRecent->actions();
            int actionIndex = recentFiles.size() - 1 - existingIndex;
            if (actionIndex >= 0 && actionIndex < actions.size())
            {
                ui->menuRecent->removeAction(actions[actionIndex]);
            }

            // Remove from list
            recentFiles.removeOne(trimmedFileName);
            addToFile = false;
            rewriteFile = true;
        }
        else
        {
            return; // Already most recent, nothing to do
        }
    }

    // Enforce maximum number of recent files
    if (recentFiles.size() >= max_num_recent_files)
    {
        // Remove oldest file (first in list)
        recentFiles.removeFirst();

        // Remove oldest action (last in menu)
        QList<QAction*> actions = ui->menuRecent->actions();
        if (!actions.isEmpty())
        {
            ui->menuRecent->removeAction(actions.last());
        }

        rewriteFile = true;
    }

    // Add to list
    recentFiles.append(trimmedFileName);

    // Add to menu (at the top)
    QAction* fileAction = new QAction(trimmedFileName, this);

    QList<QAction*> actions = ui->menuRecent->actions();
    if (!actions.isEmpty())
    {
        ui->menuRecent->insertAction(actions.first(), fileAction);
    }
    else
    {
        ui->menuRecent->addAction(fileAction);
    }

    connect(fileAction, &QAction::triggered, this, &MainWindow::on_actionRecent_triggered);

    // Write to file if needed
    if (addToFile)
    {
        QString recentFilePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
            + "/" + RECENT;
        CreateFileIfDoesNotExist(recentFilePath);

        QFile file(recentFilePath);
        if (file.open(QIODevice::Append | QIODevice::Text))
        {
            QTextStream out(&file);
            out << trimmedFileName << "\n";
            file.close();
        }
    }

    if (rewriteFile)
    {
        writeRecentFilesList();
    }
}

void MainWindow::writeRecentFilesList()
{
    QString recentFilePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
        + "/" + RECENT;

    QFile file(recentFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qWarning() << "Failed to write recent files list:" << file.errorString();
        return;
    }

    QTextStream out(&file);
    for (const QString& fileName : recentFiles)
    {
        out << fileName << "\n";
    }

    file.close();
}

QString localAppFolderAddress()
{
    QString localAppDataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    qDebug() << "Local AppData Folder:" << localAppDataPath;
    return localAppDataPath;
}

void MainWindow::on_actionRecent_triggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action)
    {
        qWarning() << "Invalid sender in on_actionRecent_triggered";
        return;
    }

    QString fileName = action->text();
    if (LoadModel(fileName))
    {
        addToRecentFiles(fileName, false);
    }
}

void MainWindow::removeFromRecentList(QAction* selectedFileAction)
{
    if (!selectedFileAction)
    {
        return;
    }

    recentFiles.removeAll(selectedFileAction->text());
    ui->menuRecent->removeAction(selectedFileAction);
    writeRecentFilesList();
}

bool MainWindow::CreateFileIfDoesNotExist(const QString& fileName)
{
    QFileInfo fileInfo(fileName);

    if (fileInfo.exists())
    {
        return true; // File already exists
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Failed to create file:" << fileName << "-" << file.errorString();
        return false;
    }

    file.close();
    return true;
}

void MainWindow::addToolBarAction(const QString& text,
    const QString& iconPath,
    const QString& toolTip,
    void (MainWindow::* slot)())
{
    QAction* action = new QAction(this);
    action->setObjectName(text);
    action->setText(text);
    action->setToolTip(toolTip);

    QIcon icon(iconPath);
    if (!icon.isNull())
    {
        action->setIcon(icon);
    }
    else
    {
        qWarning() << "Failed to load icon:" << iconPath;
    }

    ui->toolBar->addAction(action);
    connect(action, &QAction::triggered, this, slot);
}


void MainWindow::Populate_General_ToolBar()
{
    QString iconPath = qApp->applicationDirPath() + "/../../resources/Icons/";

    // Import Excel
    addToolBarAction(
        "Import Excel",
        iconPath + "Import_Excel.png",
        "Import from Excel File",
        &MainWindow::on_import_excel
    );

    // Save
    addToolBarAction(
        "Save",
        iconPath + "Save.png",
        "Save Project",
        &MainWindow::onSaveProject
    );

    // Open
    addToolBarAction(
        "Open",
        iconPath + "open.png",
        "Open Project",
        &MainWindow::onOpenProject
    );

    // Separator
    ui->toolBar->addSeparator();

    // Element Properties
    addToolBarAction(
        "Constituents Selection/Properties",
        iconPath + "Element_Props.png",
        "Constituents Selection/Properties",
        &MainWindow::on_constituent_properties_triggered
    );

    // Organic Matter and Size Correction
    addToolBarAction(
        "Organic Matter and Size Correction",
        iconPath + "OMSizeCorrection.png",
        "Organic Matter and Size Correction",
        &MainWindow::onOMSizeCorrection
    );

    // Select Samples
    addToolBarAction(
        "Select Samples",
        iconPath + "SelectSamples.png",
        "Select Samples",
        &MainWindow::onIncludeExcludeSample
    );
}

void MainWindow::on_show_data_as_table()
{
    if (!plottedData)
    {
        qWarning() << "No data to display in table";
        return;
    }

    ResultTableViewer* tableViewer = new ResultTableViewer(this);
    tableViewer->setWindowFlag(Qt::WindowMinMaxButtonsHint);
    tableViewer->setWindowTitle(tr("Elemental Profiles"));

    QTableWidget* tableWidget = plottedData->ToTable();
    tableViewer->SetTable(tableWidget);

    tableViewer->show();
}

void MainWindow::on_ZoomExtends()
{
    plotter->ZoomExtends();
}


void MainWindow::on_Options_triggered()
{
    OptionsDialog dlg(Data()->GetOptions());
    dlg.exec();

}
