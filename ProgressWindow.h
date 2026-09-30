#pragma once

#include <QWidget>
#include <QDialog>
#include "ui_ProgressWindow.h"
#include <qchartview.h>
#include <qchart.h>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include "QtCharts/QAreaSeries"
#include "progressreporter.h"

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

/**
 * @class ProgressWindow
 * @brief Graphical ProgressReporter that draws progress as charts in a dialog
 *
 * QDialog is listed first because Qt requires the QObject-derived base to come
 * first in the base list of a class using Q_OBJECT.
 */
class ProgressWindow : public QDialog, public ProgressReporter
{
	Q_OBJECT

public:
    ProgressWindow(QWidget *parent = Q_NULLPTR, int number_of_panels=1, bool extra_label_and_progressbar = false);
	~ProgressWindow();
    void AppendPoint(const double& x, const double& y, int chart=0) override;
    void SetProgress(const double& prog) override;
    void SetProgress2(const double& prog) override;
    void SetLabel(const QString& label) override;
    void SetYRange(const double &y0, const double &y1, int chart=0) override;
    void SetXRange(const double &x0, const double &x1, int chart=0) override;
    void SetTitle(const QString &title, int chart=0) override;
    void SetXAxisTitle(const QString &title, int chart=0) override;
    void SetYAxisTitle(const QString &title, int chart=0) override;
    void ClearGraph(int chart=0) override;
    void SetChartTitle(int i, const QString &title) override
    {
        if (ChartItems.size()>i)
        {   charttitles[i]=title;
            ChartItems[i].chart->setTitle(title);
        }
    }

    /// @brief Shows the dialog. Named Start() because QWidget::show() is not virtual.
    void Start() override { show(); }

    /// @brief Closes the dialog. Named Finish() because QWidget::close() is not virtual.
    void Finish() override { close(); }
private:
	Ui::ProgressWindow ui;
    QVector<ChartCollection> ChartItems;
    QVector<QString> charttitles;
};
