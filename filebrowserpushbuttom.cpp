// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#include "filebrowserpushbuttom.h"
#include "qdebug.h"
#include <QFileDialog>

FileBrowserPushButtom::FileBrowserPushButtom(QWidget *parent) : QPushButton(parent)
{
    connect(this,SIGNAL(clicked()),this,SLOT(onPress()));
}

void FileBrowserPushButtom::onPress()
{
    QString fileName;
    if (dialog_use==save_open::save)
        fileName = QFileDialog::getSaveFileName(this, tr("Save"), "", tr("Text files (*.txt);; All files (*.*)"),nullptr);
    else
        fileName = QFileDialog::getOpenFileName(this, tr("Open"), "", tr("Text files (*.txt);; All files (*.*)"),nullptr);


    setText(fileName);
}
