#pragma once

#include <QWidget>
#include <QDialog>
#include "ui_ProgressWindow.h"
#include <qchartview.h>
#include <qchart.h>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

class ProgressWindow : public QDialog
{
	Q_OBJECT

public:
	ProgressWindow(QWidget *parent = Q_NULLPTR);
	~ProgressWindow();
	void AppendPoint(const double& x, const double& y);
	void SetProgress(const double& prog);
    void SetYRange(const double &y0, const double &y1);
    void SetXRange(const double &x0, const double &x1);

private:
	Ui::ProgressWindow ui;
	QtCharts::QChart* chart;
	QtCharts::QChartView *chartView;
	QtCharts::QLineSeries* series = nullptr;
    QtCharts::QValueAxis *yaxis = nullptr;
    QtCharts::QValueAxis *xaxis = nullptr;
};
