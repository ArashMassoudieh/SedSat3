// OptionsDialog.h
#pragma once

#include <QDialog>
#include <QMap>
#include <QString>
#include <QTextEdit>
#include <QFormLayout>
#include <QDialogButtonBox>

class OptionsDialog : public QDialog {
    Q_OBJECT

public:
    explicit OptionsDialog(QMap<QString, double>* options, QWidget* parent = nullptr);

private slots:
    void applyChanges();

private:
    QMap<QString, double>* options_;
    QMap<QString, QTextEdit*> editors_;
};
