// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#ifndef RESULTSWINDOW_H
#define RESULTSWINDOW_H

#include <QDialog>
#include "string"
#include "results.h"

//using namespace std;

namespace Ui {
class ResultsWindow;
}

class ResultsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ResultsWindow(QWidget *parent = nullptr);
    void AppendText(const string &text);
    /// Adds a result's text and buttons. @p key is its key in the Results
    /// map; the buttons find the result by it.
    void AppendResult(const ResultItem &resultitem, const std::string &key);
    ~ResultsWindow();
    void SetResults(Results* res) { results = res;  }

private:
    Ui::ResultsWindow *ui;
    Results* results = nullptr;
    int result_item_counter = 0;
    /// The result a button belongs to, or nullptr if it is not in the map
    /// or holds no data
    ResultItem* ResultForButton(QObject* button);

public slots:
    void on_result_graph_clicked();
    void on_result_export_clicked();
    void on_result_table_clicked();
};

#endif // RESULTSWINDOW_H
