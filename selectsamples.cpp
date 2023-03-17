#include "selectsamples.h"
#include "ui_selectsamples.h"
#include "selectsampletablemodel.h"

SelectSamples::SelectSamples(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SelectSamples)
{
    ui->setupUi(this);
    connect(ui->GroupComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(comboChanged()));
}

SelectSamples::~SelectSamples()
{
    delete ui;
}

void SelectSamples::SetData(SourceSinkData *_data)
{
    data = _data;
    ui->GroupComboBox->clear();
    for (map<string,Elemental_Profile_Set>::iterator it=data->begin(); it!=data->end(); it++)
    {
        ui->GroupComboBox->addItem(QString::fromStdString(it->first));
    }
    SelectSampleTableModel *samplemodel = new SelectSampleTableModel(data);
    samplemodel->SetSelectedSource(ui->GroupComboBox->currentText().toStdString());
    ui->SamplestableView->setModel(samplemodel);
}

void SelectSamples::comboChanged()
{
    SelectSampleTableModel *samplemodel = new SelectSampleTableModel(data);
    samplemodel->SetSelectedSource(ui->GroupComboBox->currentText().toStdString());
    ui->SamplestableView->setModel(samplemodel);

}
