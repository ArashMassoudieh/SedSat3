#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <string>
#include <map>

#include "sourcesinkdata.h"
#include "generalplotter.h"
#include "formelementinformation.h"
#include "conductor.h"

enum ItemDataRoles {
    elementRole = Qt::ItemDataRole::UserRole + 1,
    groupRole = Qt::ItemDataRole::UserRole + 2,
    sampleRole = Qt::ItemDataRole::UserRole + 3
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @class MainWindow
 * @brief Main application window for SedSAT3 source apportionment analysis
 *
 * Provides the primary user interface for importing data, configuring analyses,
 * executing source apportionment algorithms, and visualizing results. Manages
 * data models, plotting, project file I/O, and coordinates with the Conductor
 * class for computational operations.
 */
    class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the main window
     * @param parent Parent widget (typically nullptr for main window)
     */
    explicit MainWindow(QWidget* parent = nullptr);

    /**
     * @brief Destructor - cleans up UI resources
     */
    ~MainWindow();

    /**
     * @brief Sets the sink sheet index for Excel import
     * @param i Sheet index
     */
    void SetSinkSheet(int i) { sinkSheet = i; }

    /**
     * @brief Writes a message to the application's message display area
     * @param text Message text to display
     * @param color Text color (default: black)
     */
    void WriteMessageOnScreen(const QString& text, QColor color = Qt::black);

    /**
     * @brief Returns pointer to the main data collection
     * @return Non-owning pointer to SourceSinkData
     */
    SourceSinkData* Data() { return &dataCollection; }
    const SourceSinkData* Data() const { return &dataCollection; }
    /**
     * @brief Executes a specified analysis command
     * @param command Command name identifying the analysis type
     * @param arguments Map of parameter names to values
     * @return true if execution successful, false otherwise
     */
    bool Execute(const std::string& command, std::map<std::string, std::string> arguments);

private:
    // UI Setup methods
    /**
     * @brief Sets up all signal-slot connections for UI elements
     *
     * Connects menu actions, toolbar buttons, and tree views to their
     * respective slot handlers. Uses modern Qt5 function pointer syntax
     * for type safety.
     */
    void setupConnections();

    /**
     * @brief Initializes the tools tree view
     *
     * Loads tool definitions from JSON configuration file and populates
     * the tools tree view with available analysis operations.
     */
    void setupToolsView();

    /**
     * @brief Initializes the results tree view and context menu
     *
     * Sets up the results tree view model and configures the context menu
     * for result management (delete, etc.).
     */
    void setupResultsView();

    /**
     * @brief Initializes the general toolbar
     *
     * Configures toolbar buttons with icons and initial enabled states.
     */
    void setupToolBar();

    // Data import/export methods
    /**
     * @brief Reads elemental profile data from Excel file
     * @param filename Path to Excel file
     * @return true if import successful, false otherwise
     */
    bool ReadExcel(const QString& filename);

    // Model conversion methods
    QStandardItemModel* ToQStandardItemModel(const SourceSinkData* srcsinkdata);
    QStandardItem* ToQStandardItem(const QString& key, const SourceSinkData* srcsinkdata);
    QStandardItem* ElementsToQStandardItem(const QString& key, const SourceSinkData* srcsinkdata);
    QStandardItem* ToQStandardItem(const QString& key, const QJsonObject& json);
    QStandardItemModel* ToQStandardItemModel(const QJsonDocument& jsondocument);

    QString TreeQStringSelectedType();

    // JSON file I/O
    QJsonDocument loadJson(const QString& fileName);
    void saveJson(const QJsonDocument& document, const QString& fileName);

    // Table initialization
    void InitiateTables();

    // Recent files management
    void readRecentFilesList();
    void addToRecentFiles(const QString &fileName, bool addToFile);
    void writeRecentFilesList();
    void removeFromRecentList(QAction* selectedFileAction);
    bool CreateFileIfDoesNotExist(const QString &fileName);

    // Project file management
    bool LoadModel(const QString& fileName);

    // Toolbar population
    void Populate_General_ToolBar();

private slots:
    // File operations
    void on_import_excel();

    // Plotting
    void on_plot_raw_elemental_profiles();

    // Tree view handling
    void on_tree_selectionChanged(const QItemSelection& changed);
    void preparetreeviewMenu(const QPoint& pos);
    void showdistributionsforelements();

    // Data management
    void on_constituent_properties_triggered();
    void onIncludeExcludeSample();
    void onOMSizeCorrection();

    // Tool execution
    void on_tool_executed(const QModelIndex& index);
    void on_old_result_requested(const QModelIndex& index);

    // Application menu
    void onAboutTriggered();
    void on_Options_triggered();

    
private:
    // Project management
    /**
        * @brief Gets the file path for saving, prompting user if needed
        * @param promptUser If true, always show save dialog; if false, use existing ProjectFileName
        * @return File path for saving, or empty string if cancelled
        */
    QString getSaveFilePath(bool promptUser);

    /**
        * @brief Saves project data to the specified file
        * @param filePath Path to save the project file
        * @return true if save successful, false otherwise
        */
    bool saveProjectToFile(const QString& filePath);

    /**
        * @brief Builds the complete project JSON object
        * @return JSON object containing all project data
        */
    QJsonObject buildProjectJson() const;

    /**
        * @brief Builds the results section of the project JSON
        * @return JSON object containing all results
        */
    QJsonObject buildResultsJson() const;
    
	void onSaveProject();
    void onSaveProjectAs();
    void onOpenProject();
    void on_actionRecent_triggered();

    // Results management
    void DeleteResults();
    void onCustomContextMenu(const QPoint& pos);

    // Display
    void on_show_data_as_table();
    void on_ZoomExtends();

    /**
     * @brief Helper to add an action to the toolbar with icon and connection
     *
     * Creates a toolbar action with the specified text, icon, and tooltip,
     * then connects it to the given member function slot.
     *
     * @param text Action text and object name
     * @param iconPath Path to icon file relative to resources folder
     * @param toolTip Tooltip text displayed on hover
     * @param slot Member function pointer to connect to triggered signal
     */
    void addToolBarAction(const QString& text,
        const QString& iconPath,
        const QString& toolTip,
        void (MainWindow::* slot)());

    Ui::MainWindow* ui;

    // Data
    SourceSinkData dataCollection;                      ///< Main data collection for analysis
    int sinkSheet = -1;                                 ///< Index of sink sheet in Excel import

    // Visualization
    GeneralPlotter* plotter = nullptr;                  ///< Main plotting widget
    std::unique_ptr<Interface> plottedData;             ///< Currently plotted data interface

    // Models
    std::unique_ptr<QStandardItemModel> columnViewModel; ///< Model for column/data tree view
    std::unique_ptr<QStandardItemModel> resultsViewModel;///< Model for results tree view

    // Context menus and actions
    QMenu* ResultscontextMenu = nullptr;                ///< Context menu for results view
    QAction* DeleteAction = nullptr;                    ///< Delete action for results

    // JSON configuration
    QJsonDocument formsstructure;                       ///< Forms structure definitions

    // State tracking
    QString selectedTreeItemType = "None";              ///< Currently selected tree item type
    bool treeItemChangedProgramatically = false;        ///< Flag for programmatic tree changes
    QString ProjectFileName;                             ///< Current project file path
    QStringList recentFiles;                            ///< List of recently opened files

    // Main components
    std::unique_ptr<QMenu> menu;                        ///< Main menu
    std::unique_ptr<QWidget> centralform;               ///< Central form widget
    std::unique_ptr<Conductor> conductor;               ///< Analysis command coordinator
    QModelIndex indexresultselected;                    ///< Currently selected result index
};

/**
 * @brief Returns the local application folder address
 * @return Path to local application data folder
 */
QString localAppFolderAddress();

#endif // MAINWINDOW_H