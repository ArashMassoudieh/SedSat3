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
};

#endif // FORMELEMENTINFORMATION_H
