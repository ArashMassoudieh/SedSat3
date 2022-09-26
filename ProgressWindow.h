#pragma once

#include <QWidget>
#include "ui_ProgressWindow.h"
#include <qchartview.h>
#include <qchart.h>

class ProgressWindow : public QWidget
{
	Q_OBJECT

public:
	ProgressWindow(QWidget *parent = Q_NULLPTR);
	~ProgressWindow();

private:
	Ui::ProgressWindow ui;
	QtCharts::QChart* chart;
	QtCharts::QChart *progress_chart; 
};
