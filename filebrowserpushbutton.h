// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#ifndef FILEBROWSERPUSHBUTTON_H
#define FILEBROWSERPUSHBUTTON_H

#include <QPushButton>

#include <QObject>

enum class save_open {open, save};

class FileBrowserPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit FileBrowserPushButton(QWidget *parent = nullptr);
    save_open dialog_use = save_open::save;

signals:

public slots:
    void onPress();

};

#endif // FILEBROWSERPUSHBUTTON_H
