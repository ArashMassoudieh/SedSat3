// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#pragma once

#include <QObject>
#include <qpushbutton.h>

class FilePushButton : public QPushButton
{
	Q_OBJECT
public:
	FilePushButton(QWidget *parent);
	~FilePushButton();
public slots:
	void browserClicked();
};
