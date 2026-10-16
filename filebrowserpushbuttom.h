// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#ifndef FILEBROWSERPUSHBUTTOM_H
#define FILEBROWSERPUSHBUTTOM_H

#include <QPushButton>

#include <QObject>

enum class save_open {open, save};

class FileBrowserPushButtom : public QPushButton
{
    Q_OBJECT
public:
    explicit FileBrowserPushButtom(QWidget *parent = nullptr);
    save_open dialog_use = save_open::save;

signals:

public slots:
    void onPress();

};

#endif // FILEBROWSERPUSHBUTTOM_H
