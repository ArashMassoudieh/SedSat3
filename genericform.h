#ifndef GENERICFORM_H
#define GENERICFORM_H

#include <QWidget>
#include "QJsonObject"


enum class delegate_type {LineEdit, ComboBox, CheckBox, SpinBix};
struct parameter_property
{
    QString Discription;
    delegate_type Type = delegate_type::LineEdit;
    QWidget *InputWidget=nullptr;
    QString DefaultValue;
};

namespace Ui {
class GenericForm;
}

class GenericForm : public QWidget
{
    Q_OBJECT

public:
    explicit GenericForm(QJsonObject *formdata, QWidget *parent = nullptr);
    QVector<parameter_property> Parameter_Properties;
    ~GenericForm();

private:
    Ui::GenericForm *ui;
};

#endif // GENERICFORM_H
