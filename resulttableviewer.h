// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#ifndef RESULTTABLEVIEWER_H
#define RESULTTABLEVIEWER_H

#include <QDialog>
#include <QTableWidget>

namespace Ui {
class ResultTableViewer;
}

class ResultTableViewer : public QDialog
{
    Q_OBJECT

public:
    explicit ResultTableViewer(QWidget *parent = nullptr);
    void SetTable(QTableWidget *tablewidget);
    ~ResultTableViewer();

private:
    Ui::ResultTableViewer *ui;
    QTableWidget *tablewidget;
public slots:
    void on_ExportToCSV();
};

#endif // RESULTTABLEVIEWER_H
