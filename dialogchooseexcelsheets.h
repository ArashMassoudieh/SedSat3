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
