// OptionsDialog.cpp
#include "optionsdialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

OptionsDialog::OptionsDialog(QMap<QString, double>* options, QWidget* parent)
    : QDialog(parent), options_(options)
{
    setWindowTitle("SedSat Options");

    auto* layout = new QVBoxLayout(this);
    auto* formLayout = new QFormLayout;

    // Create a QTextEdit for each option
    for (auto it = options_->begin(); it != options_->end(); ++it) {
        auto* edit = new QTextEdit(QString::number(it.value()));
        edit->setFixedHeight(30); // like a line edit
        editors_[it.key()] = edit;
        formLayout->addRow(new QLabel(it.key()), edit);
    }

    layout->addLayout(formLayout);

    // OK/Cancel buttons
    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &OptionsDialog::applyChanges);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &OptionsDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &OptionsDialog::reject);
}

void OptionsDialog::applyChanges() {
    for (auto it = editors_.begin(); it != editors_.end(); ++it) {
        bool ok = false;
        double val = it.value()->toPlainText().toDouble(&ok);
        if (ok) {
            (*options_)[it.key()] = val;
        }
    }
}
