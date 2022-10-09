#ifndef RESULTSWINDOW_H
#define RESULTSWINDOW_H

#include <QDialog>
#include "string"

using namespace std;

namespace Ui {
class ResultsWindow;
}

class ResultsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ResultsWindow(QWidget *parent = nullptr);
    void AppendText(const string &text);
    ~ResultsWindow();

private:
    Ui::ResultsWindow *ui;
};

#endif // RESULTSWINDOW_H
