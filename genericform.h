#ifndef GENERICFORM_H
#define GENERICFORM_H

#include <QWidget>
#include "QJsonObject"
#include "QPushButton"

class MainWindow;

enum class delegate_type {LineEdit, ComboBox, CheckBox, SpinBix};
struct parameter_property
{
    QString Discription;
    delegate_type Type = delegate_type::LineEdit;
    QWidget *InputWidget=nullptr;
    QString DefaultValue;
    QString value;
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
    QPushButton *buttonOk = nullptr;
    QPushButton *buttonCancel = nullptr;
    Ui::GenericForm *ui;
    MainWindow *mainwindow();
public slots:
    void onProceed();
    void onCancel();
};

#endif // GENERICFORM_H
