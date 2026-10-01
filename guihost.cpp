#include "guihost.h"
#include "mainwindow.h"
#include "ProgressWindow.h"

#include <QMessageBox>
#include <QString>

GuiHost::GuiHost(MainWindow* _mainwindow)
    : mainwindow(_mainwindow)
{
}

GuiHost::~GuiHost()
{
    // Anything still outstanding belongs to an analysis that did not release
    // it. Close and delete it rather than leaving the dialog on screen.
    for (ProgressWindow* window : progress_windows)
    {
        if (window)
        {
            window->close();
            delete window;
        }
    }
    progress_windows.clear();
}

void GuiHost::ShowWarning(const std::string& message)
{
    QMessageBox::warning(mainwindow,
                         "SedSAT3",
                         QString::fromStdString(message),
                         QMessageBox::Ok);
}

ProgressReporter* GuiHost::CreateProgressReporter(int number_of_panels,
                                                  bool extra_label_and_progressbar)
{
    ProgressWindow* window = new ProgressWindow(mainwindow,
                                                number_of_panels,
                                                extra_label_and_progressbar);
    progress_windows.append(window);
    return window;
}

void GuiHost::DisposeProgressReporter(ProgressReporter* reporter)
{
    if (reporter == nullptr)
        return;

    for (int i = 0; i < progress_windows.size(); i++)
    {
        if (progress_windows[i] == reporter)
        {
            ProgressWindow* window = progress_windows[i];
            progress_windows.remove(i);
            window->close();
            delete window;
            return;
        }
    }
}
