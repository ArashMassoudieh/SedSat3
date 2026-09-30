#ifndef GUIHOST_H
#define GUIHOST_H

#include "analysishost.h"

#include <QVector>

class MainWindow;
class ProgressWindow;

/**
 * @class GuiHost
 * @brief AnalysisHost that reports through the graphical interface
 *
 * Warnings are shown as message boxes and progress is drawn in
 * ProgressWindow dialogs, both parented to the MainWindow. This is the
 * behaviour the application had before the Conductor was separated from the
 * interface it runs in.
 *
 * @see ConsoleHost for the command line counterpart
 */
class GuiHost : public AnalysisHost
{
public:
    /**
     * @brief Constructs a host that reports through @p mainwindow
     * @param mainwindow Parent for message boxes and progress dialogs
     *        (non-owning, must outlive this host)
     */
    explicit GuiHost(MainWindow* mainwindow);

    /// @brief Closes and deletes any progress windows still outstanding
    ~GuiHost() override;

    /// @brief Shows @p message in a warning dialog
    void ShowWarning(const std::string& message) override;

    /// @brief Creates a ProgressWindow and shows nothing until Start() is called
    ProgressReporter* CreateProgressReporter(int number_of_panels = 1,
                                             bool extra_label_and_progressbar = false) override;

    /// @brief Closes and deletes a progress window created by this host
    void DisposeProgressReporter(ProgressReporter* reporter) override;

private:
    MainWindow* mainwindow;                   ///< Non-owning parent window
    QVector<ProgressWindow*> progress_windows; ///< Outstanding progress dialogs, owned
};

#endif // GUIHOST_H
