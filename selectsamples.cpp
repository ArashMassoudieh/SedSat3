#include "selectsamples.h"
#include "ui_selectsamples.h"
#include "selectsampletablemodel.h"
#include "omsizecorrectiontablemodel.h"

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
        if (Mode!=mode::regressions || it->first!=data->TargetGroup())
            ui->GroupComboBox->addItem(QString::fromStdString(it->first));
    }
    if (Mode==mode::samples)
    {   SelectSampleTableModel *samplemodel = new SelectSampleTableModel(data);
        samplemodel->SetSelectedSource(ui->GroupComboBox->currentText().toStdString());
        ui->SamplestableView->setModel(samplemodel);
        SelectSampleDelegate *samplesDelegate = new SelectSampleDelegate(data, this);
        samplesDelegate->SetMode(mode::samples);
        ui->SamplestableView->setItemDelegate(samplesDelegate);
    }
    else if (Mode==mode::regressions)
    {
        OmSizeCorrectionTableModel *OMSizemodel = new OmSizeCorrectionTableModel(data->operator[](ui->GroupComboBox->currentText().toStdString()).GetExistingRegressionSet());
        ui->SamplestableView->setModel(OMSizemodel);
        SelectSampleDelegate *omsizecorrectionDelegate = new SelectSampleDelegate(data, this);
        omsizecorrectionDelegate->SetMode(mode::regressions);
        ui->SamplestableView->setItemDelegate(omsizecorrectionDelegate);
    }


}

void SelectSamples::comboChanged()
{
    if (Mode==mode::samples)
    {   SelectSampleTableModel *samplemodel = new SelectSampleTableModel(data);
        samplemodel->SetSelectedSource(ui->GroupComboBox->currentText().toStdString());
        ui->SamplestableView->setModel(samplemodel);
    }
    else if (Mode==mode::regressions)
    {
        OmSizeCorrectionTableModel *OMSizemodel = new OmSizeCorrectionTableModel(data->operator[](ui->GroupComboBox->currentText().toStdString()).GetExistingRegressionSet());
        ui->SamplestableView->setModel(OMSizemodel);
        SelectSampleDelegate *omsizecorrectionDelegate = new SelectSampleDelegate(data, this);
        omsizecorrectionDelegate->SetMode(mode::regressions);
        ui->SamplestableView->setItemDelegate(omsizecorrectionDelegate);
    }


}
