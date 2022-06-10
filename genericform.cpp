#include "genericform.h"
#include "ui_genericform.h"


GenericForm::GenericForm(QJsonObject *formdata, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GenericForm)
{
    ui->setupUi(this);

    for (int i=0; i<formdata->keys().size(); i++)
    {
        if (formdata->value(formdata->keys()[i]).isObject())
        {
            QJsonObject object = formdata->value(formdata->keys()[i]).toObject();
            formdata->value(formdata->keys()[i]).toObject();
        }
        else
        {
            QStandardItem *subitem = new QStandardItem(json.keys()[i]);
            standarditem->appendRow(subitem);
        }
    }
    return standarditem;
}

GenericForm::~GenericForm()
{
    delete ui;
}
