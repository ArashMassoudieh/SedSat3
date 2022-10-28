#include "ProgressWindow.h"
#include "QtCharts/QAreaSeries"


ProgressWindow::ProgressWindow(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	series = new QtCharts::QLineSeries();
	chart = new QtCharts::QChart();
	chart->legend()->hide();
	chart->setTitle("Optimization progress");
	QtCharts::QAreaSeries* areaseries = new QtCharts::QAreaSeries(series);
	chartView = new QtCharts::QChartView(chart);
	chartView->setRenderHint(QPainter::Antialiasing);
	chart->addSeries(areaseries);
	chart->addSeries(series);
	

	
    QPen pen(0x059605);
	pen.setWidth(3);
	series->setPen(pen);

	QLinearGradient gradient(QPointF(0, 0), QPointF(0, 1));
	gradient.setColorAt(0.0, 0x3cc63c);
	gradient.setColorAt(1.0, 0x26f626);
	//gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
	series->setBrush(gradient);
    yaxis = new QtCharts::QValueAxis();
    xaxis = new QtCharts::QValueAxis();

    chart->addAxis(xaxis,Qt::AlignBottom);
    chart->addAxis(yaxis,Qt::AlignLeft);
    series->attachAxis(xaxis);
    series->attachAxis(yaxis);
	areaseries->attachAxis(xaxis);
	areaseries->attachAxis(yaxis);
    //chart->createDefaultAxes();


	ui.horizontalLayout->addWidget(chartView);
	
}

ProgressWindow::~ProgressWindow()
{
}

void ProgressWindow::AppendPoint(const double& x, const double& y)
{
	series->append(x, y);
    if (y<yaxis->min())
        yaxis->setMin(y);
	chartView->update();
}

void ProgressWindow::SetProgress(const double& prog)
{
	ui.progressBar->setValue(prog * 100);
}

void ProgressWindow::SetYRange(const double &y0, const double &y1)
{
    yaxis->setMin(y0);
	yaxis->setMax(y1);
    yaxis->setTitleText("Fitness");
    chartView->update();
	
}

void ProgressWindow::SetXRange(const double &x0, const double &x1)
{
	xaxis->setMin(x0);
	xaxis->setMax(x1);
    xaxis->setTitleText("Generation");
    chartView->update();
}
