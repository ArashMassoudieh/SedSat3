// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#ifndef INDICATESHEETSDIALOG_H
#define INDICATESHEETSDIALOG_H

#include <QDialog>
#include <QRadioButton>
#include <QLabel>



namespace Ui {
class IndicateSheetsDialog;
}

class IndicateSheetsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IndicateSheetsDialog(QWidget *parent = nullptr);
    ~IndicateSheetsDialog();
    void Populate_Table(const QStringList &sheets);

private:
    Ui::IndicateSheetsDialog *ui;
    QList<QRadioButton*> radio_buttons_sinks;
    QList<QRadioButton*> radio_buttons_sources;
    QStringList group_names;
    QList<QLabel*> sheet_labels;
    QLabel* summary_label = nullptr;
    /// Shows the target sheet's name in bold, and names it above the list
    void UpdateTargetHighlight();
private slots:
    void reject();
    void accept();
    void on_radio_button_changed();
};

#endif // INDICATESHEETSDIALOG_H
