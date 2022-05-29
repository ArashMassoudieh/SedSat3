#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QDialog>
#include "sourcesinkdata.h"
#include "QComboBox"
#include "customplotbar.h"

namespace Ui {
class PlotWindow;
}

class PlotWindow : public QDialog
{
    Q_OBJECT

public:
    explicit PlotWindow(QWidget *parent = nullptr);
    ~PlotWindow();
    SourceSinkData *Data();

private:
    Ui::PlotWindow *ui;
    QComboBox *combo_groups = nullptr;
    QComboBox *combo_sample = nullptr;
    CustomPlotBar *plot = nullptr;
    void PlotData();

public slots:
    void on_combo_group_changed();
    void on_combo_sample_changed();
};

#endif // PLOTWINDOW_H
