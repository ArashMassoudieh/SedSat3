#ifndef INDICATESHEETSDIALOG_H
#define INDICATESHEETSDIALOG_H

#include <QDialog>
#include <QRadioButton>



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
private slots:
    void reject();
    void accept();
    void on_radio_button_changed();
};

#endif // INDICATESHEETSDIALOG_H
