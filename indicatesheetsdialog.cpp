// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#include "indicatesheetsdialog.h"
#include "ui_indicatesheetsdialog.h"
#include "qlabel.h"
#include "qgroupbox.h"
#include "QRadioButton"
#include "QDebug"
#include "mainwindow.h"
#include <QMessageBox>
#include <QVBoxLayout>

IndicateSheetsDialog::IndicateSheetsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IndicateSheetsDialog)
{
    ui->setupUi(this);
    // Which sheet is the target was hard to see from the radio buttons alone
    // (domain review): it is named here and shown in bold in the list
    summary_label = new QLabel(this);
    summary_label->setWordWrap(true);
    if (auto* layout = qobject_cast<QVBoxLayout*>(this->layout()))
        layout->insertWidget(0, summary_label);
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
        sheet_labels.append(label);
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
    UpdateTargetHighlight();
}

void IndicateSheetsDialog::UpdateTargetHighlight()
{
    QString target;
    for (int i = 0; i < sheet_labels.count(); i++)
    {
        const bool is_target = radio_buttons_sinks[i]->isChecked();
        sheet_labels[i]->setText(is_target ? group_names[i] + "  (target)" : group_names[i]);
        sheet_labels[i]->setStyleSheet(is_target ? "font-weight: bold; color: #0072B2;" : "");
        if (is_target)
            target = group_names[i];
    }
    if (summary_label != nullptr)
        summary_label->setText(target.isEmpty()
            ? tr("Choose the sheet that holds the target samples: select <b>Target</b> on its row. Every other sheet is a source group.")
            : tr("Target samples: <b>%1</b>. Every other sheet is a source group.").arg(target.toHtmlEscaped()));
}

void IndicateSheetsDialog::reject()
{
    QDialog::reject();
}
void IndicateSheetsDialog::accept()
{
    bool target_chosen = false;
    for (QRadioButton* button : radio_buttons_sinks)
        target_chosen = target_chosen || button->isChecked();
    if (!target_chosen)
    {
        // Without a target sheet the import used to go ahead with none
        QMessageBox::warning(this, tr("Target sheet"), tr("Select the sheet that holds the target samples."));
        return;
    }
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
    UpdateTargetHighlight();
}
