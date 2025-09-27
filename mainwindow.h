#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "sourcesinkdata.h"
#include "generalplotter.h"
#include "formelementinformation.h"
#include "conductor.h"


enum ItemDataRoles {elementRole = Qt::ItemDataRole::UserRole+1, groupRole = Qt::ItemDataRole::UserRole+2, sampleRole = Qt::ItemDataRole::UserRole+3};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void SetSinkSheet(int i) {Sink_Sheet = i;}
    void WriteMessageOnScreen(const QString &text, QColor color=Qt::black);
    SourceSinkData *Data() {return &DataCollection;}
    bool Execute(const string &command, map<string,string> arguments);
private:
    Ui::MainWindow *ui;
    int Sink_Sheet=-1;
    bool ReadExcel(const QString &filename);
    SourceSinkData DataCollection;
    GeneralPlotter *plotter = nullptr;
    QStandardItemModel* ToQStandardItemModel(const SourceSinkData* srcsinkdata);
    QStandardItem* ToQStandardItem(const QString &key, const SourceSinkData* srcsinkdata);
    QStandardItem* ElementsToQStandardItem(const QString &key, const SourceSinkData* srcsinkdata);
    QString TreeQStringSelectedType();
    QStandardItem* ToQStandardItem(const QString &key, const QJsonObject &json);
    QStandardItemModel* ToQStandardItemModel(const QJsonDocument &jsondocument);
    QStandardItemModel *columnviewmodel = nullptr;
    QStandardItemModel *resultsviewmodel = nullptr;
    QMenu* ResultscontextMenu = nullptr;
    QAction* DeleteAction;
    QJsonDocument loadJson(const QString &fileName);
    void saveJson(const QJsonDocument &document, const QString &fileName);
    QString SelectedTreeItemType = "None";
    bool treeitemchangedprogramatically = false;
    std::unique_ptr<QMenu> menu;
    QJsonDocument formsstructure;
    std::unique_ptr<QWidget> centralform = nullptr;
    Conductor *conductor=nullptr;
    QModelIndex indexresultselected;
    void InitiateTables();
    QString ProjectFileName;
    void readRecentFilesList();
    void addToRecentFiles(QString fileName, bool addToFile);
    void writeRecentFilesList();
    void removeFromRecentList(QAction* selectedFileAction);
    QStringList recentFiles;
    bool CreateFileIfDoesNotExist(QString fileName);
    bool LoadModel(const QString &fileName);
    void Populate_General_ToolBar();
    Interface *plotted_data = nullptr;
private slots:
    void on_import_excel();
    void on_plot_raw_elemental_profiles();
    void on_test_plot();
    void on_tree_selectionChanged(const QItemSelection &changed);
    void preparetreeviewMenu(const QPoint &pos);
    void showdistributionsforelements();
    void on_constituent_properties_triggered();
    void on_test_dialog_triggered();
    void on_tool_executed(const QModelIndex&);
    void on_old_result_requested(const QModelIndex&);
    void on_test_likelihood();
    void on_test_progress_window();
    void on_TestLevenberg_Marquardt();
    void onAboutTriggered();
    void onSaveProject();
    void onSaveProjectAs();
    void onOpenProject();
    void onIncludeExcludeSample();
    void onOMSizeCorrection();
    void onCustomContextMenu(const QPoint &);
    void on_actionRecent_triggered();
    void DeleteResults();
    void on_show_data_as_table();
    void on_ZoomExtends();
    void on_Options_triggered();


};

QString localAppFolderAddress();
#endif // MAINWINDOW_H
