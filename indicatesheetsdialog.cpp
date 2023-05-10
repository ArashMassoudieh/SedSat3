#include "indicatesheetsdialog.h"
#include "ui_indicatesheetsdialog.h"
#include "qlabel.h"
#include "qgroupbox.h"
#include "QRadioButton"
#include "QDebug"
#include "mainwindow.h"

IndicateSheetsDialog::IndicateSheetsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IndicateSheetsDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

IndicateSheetsDialog::~IndicateSheetsDialog()
{
    delete ui;
}

void IndicateSheetsDialog::Populate_Table(const QStringList &sheets)
{
    for (int i=0; i<sheets.count(); i++)
    {
        QLabel *label = new QLabel(ui->scrollAreaWidgetContents);
        label->setText(sheets[i]);
        group_names<<sheets[i];
        ui->formLayout->setWidget(i, QFormLayout::LabelRole, label);

        QGroupBox *groupBox = new QGroupBox(ui->scrollAreaWidgetContents);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));

        QHBoxLayout *horizontalLayout = new QHBoxLayout(groupBox);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        QRadioButton* radioButton_mixture = new QRadioButton(groupBox);
        radioButton_mixture->setObjectName(QString::fromUtf8("Target"));
        radioButton_mixture->setText("Target");
        horizontalLayout->addWidget(radioButton_mixture);
        radio_buttons_sinks.append(radioButton_mixture);
        QRadioButton* radioButton_source = new QRadioButton(groupBox);
        radioButton_source->setObjectName(QString::fromUtf8("Source"));
        radioButton_source->setText("Source");
        horizontalLayout->addWidget(radioButton_source);
        radio_buttons_sources.append(radioButton_source);

        radioButton_source->setChecked(i!=0);
        radioButton_mixture->setChecked(i==0);
        connect(radioButton_mixture,SIGNAL(clicked()),this, SLOT(on_radio_button_changed()));
        connect(radioButton_source,SIGNAL(clicked()),this, SLOT(on_radio_button_changed()));
        ui->formLayout->setWidget(i, QFormLayout::FieldRole, groupBox);

    }
}

void IndicateSheetsDialog::reject()
{
    QDialog::reject();
}
void IndicateSheetsDialog::accept()
{
    for (int i=0; i<radio_buttons_sinks.count(); i++)
    {
        if (radio_buttons_sinks[i]->isChecked())
        {   dynamic_cast<MainWindow*>(parent())->SetSinkSheet(i);
            dynamic_cast<MainWindow*>(parent())->Data()->SetTargetGroup(group_names[i].toStdString());
        }
    }
    QDialog::accept();
}

void IndicateSheetsDialog::on_radio_button_changed()
{
    if (dynamic_cast<QRadioButton*>(sender())->text()=="Target" && dynamic_cast<QRadioButton*>(sender())->isChecked())
    {
        for (int i=0; i<radio_buttons_sinks.count(); i++)
        {
            if (radio_buttons_sinks[i]!=sender())
            {
                radio_buttons_sinks[i]->setChecked(false);
                radio_buttons_sources[i]->setChecked(true);
            }

        }
    }
}
