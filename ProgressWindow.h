#pragma once

#include <QWidget>
#include <QDialog>
#include "ui_ProgressWindow.h"
#include <qchartview.h>
#include <qchart.h>
#include <QtCharts/QLineSeries>

class ProgressWindow : public QDialog
{
	Q_OBJECT

public:
	ProgressWindow(QWidget *parent = Q_NULLPTR);
	~ProgressWindow();
	void AppendPoint(const double& x, const double& y);
	void SetProgress(const double& prog);

private:
	Ui::ProgressWindow ui;
	QtCharts::QChart* chart;
	QtCharts::QChartView *chartView;
	QtCharts::QLineSeries* series = nullptr;
};
