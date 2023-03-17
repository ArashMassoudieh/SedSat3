#include "selectsamples.h"
#include "ui_selectsamples.h"

SelectSamples::SelectSamples(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SelectSamples)
{
    ui->setupUi(this);
}

SelectSamples::~SelectSamples()
{
    delete ui;
}

void SelectSamples::SetData(SourceSinkData *_data)
{
    data = _data;

}
