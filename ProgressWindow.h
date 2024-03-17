#pragma once

#include <QWidget>
#include <QDialog>
#include "ui_ProgressWindow.h"
#include <qchartview.h>
#include <qchart.h>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include "QtCharts/QAreaSeries"

#ifndef Qt6
using namespace QtCharts;
#endif

enum class progress_window_mode {single_panel, double_panel};

struct ChartCollection
{
    QChart* chart;
    QChartView *chartView;
    QLineSeries* series;
    QAreaSeries* areaseries;
    QValueAxis *yaxis;
    QValueAxis *xaxis;
};

class ProgressWindow : public QDialog
{
	Q_OBJECT

public:
    ProgressWindow(QWidget *parent = Q_NULLPTR, int number_of_panels=1, bool extra_label_and_progressbar = false);
	~ProgressWindow();
    void AppendPoint(const double& x, const double& y, int chart=0);
    void SetProgress(const double& prog);
    void SetProgress2(const double& prog);
    void SetLabel(const QString& label);
    void SetYRange(const double &y0, const double &y1, int chart=0);
    void SetXRange(const double &x0, const double &x1, int chart=0);
    void SetTitle(const QString &title, int chart=0);
    void SetXAxisTitle(const QString &title, int chart=0);
    void SetYAxisTitle(const QString &title, int chart=0);
    void ClearGraph(int chart=0);

private:
	Ui::ProgressWindow ui;
    QVector<ChartCollection> ChartItems;
};
