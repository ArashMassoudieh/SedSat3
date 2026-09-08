#ifndef FORMELEMENTINFORMATION_H
#define FORMELEMENTINFORMATION_H

#include <QWidget>
#include <QTableView>

namespace Ui {
class FormElementInformation;
}

class FormElementInformation : public QWidget
{
    Q_OBJECT

public:
    explicit FormElementInformation(QWidget *parent = nullptr);
    ~FormElementInformation();
    QTableView *table();
private:
    Ui::FormElementInformation *ui;

private slots:
    void on_Include_Exclude_Change();
};

#endif // FORMELEMENTINFORMATION_H
