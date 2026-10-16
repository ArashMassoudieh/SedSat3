// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#ifndef DIALOGCHOOSEEXCELSHEETS_H
#define DIALOGCHOOSEEXCELSHEETS_H

#include <QDialog>

namespace Ui {
class DialogChooseExcelSheets;
}

class DialogChooseExcelSheets : public QDialog
{
    Q_OBJECT

public:
    explicit DialogChooseExcelSheets(QWidget *parent, QStringList **_items);
    ~DialogChooseExcelSheets();
    void AddItem(const QString &item);

private:
    Ui::DialogChooseExcelSheets *ui;
    QStringList *items = nullptr;
private slots:
    void reject();
    void accept();
};

#endif // DIALOGCHOOSEEXCELSHEETS_H
