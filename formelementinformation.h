// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#ifndef FORMELEMENTINFORMATION_H
#define FORMELEMENTINFORMATION_H

#include <QWidget>
#include <QTableView>

class ElementTableModel;

namespace Ui {
class FormElementInformation;
}

class FormElementInformation : public QWidget
{
    Q_OBJECT

public:
    explicit FormElementInformation(QWidget *parent = nullptr);
    ~FormElementInformation();
    QTableView *table();
    void setModel(ElementTableModel *model); ///< Sets the table model and the check box state from it
private:
    Ui::FormElementInformation *ui;

private slots:
    void OnIncludeAllToggled();
    void SyncIncludeAllCheckBox(); ///< Checked exactly when every element is included
};

#endif // FORMELEMENTINFORMATION_H
