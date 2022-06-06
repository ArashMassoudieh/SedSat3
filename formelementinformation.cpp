#include "formelementinformation.h"
#include "ui_formelementinformation.h"

FormElementInformation::FormElementInformation(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormElementInformation)
{
    ui->setupUi(this);
}

FormElementInformation::~FormElementInformation()
{
    delete ui;
}
