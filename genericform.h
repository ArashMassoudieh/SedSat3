#ifndef GENERICFORM_H
#define GENERICFORM_H

#include <QWidget>
#include "QJsonObject"

namespace Ui {
class GenericForm;
}

class GenericForm : public QWidget
{
    Q_OBJECT

public:
    explicit GenericForm(QJsonObject *formdata, QWidget *parent = nullptr);
    ~GenericForm();

private:
    Ui::GenericForm *ui;
};

#endif // GENERICFORM_H
