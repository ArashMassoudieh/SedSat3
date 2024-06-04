#include "ProgressWindow.h"



ProgressWindow::ProgressWindow(QWidget *parent, int number_of_panels, bool extra_label_and_progressbar)
	: QDialog(parent)
{
	ui.setupUi(this);
    ChartItems.resize(number_of_panels);
    charttitles.resize(number_of_panels);
    if (!extra_label_and_progressbar)
    {
        ui.progressBar_2->setVisible(false);
        ui.label->setVisible(false);
    }
    else
    {
        ui.progressBar_2->setVisible(true);
        ui.label->setVisible(true);
    }

    for (int i=0; i<number_of_panels; i++)
    {
        ChartItems[i].series = new QLineSeries();
        ChartItems[i].chart = new QChart();
        ChartItems[i].chart->legend()->hide();
        ChartItems[i].chart->setTitle("Optimization progress");
        ChartItems[i].areaseries = new QAreaSeries(ChartItems[i].series);
        ChartItems[i].chartView = new QChartView(ChartItems[i].chart);
        ChartItems[i].chartView->setRenderHint(QPainter::Antialiasing);
        ChartItems[i].chart->addSeries(ChartItems[i].areaseries);
        ChartItems[i].chart->addSeries(ChartItems[i].series);

        QPen pen(0x059605);
        pen.setWidth(3);
        ChartItems[i].series->setPen(pen);

        QLinearGradient gradient(QPointF(0, 0), QPointF(0, 1));
        gradient.setColorAt(0.0, 0x3cc63c);
        gradient.setColorAt(1.0, 0x26f626);
        //gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        ChartItems[i].series->setBrush(gradient);
        ChartItems[i].yaxis = new QValueAxis();
        ChartItems[i].xaxis = new QValueAxis();

        ChartItems[i].chart->addAxis(ChartItems[i].xaxis,Qt::AlignBottom);
        ChartItems[i].chart->addAxis(ChartItems[i].yaxis,Qt::AlignLeft);
        ChartItems[i].series->attachAxis(ChartItems[i].xaxis);
        ChartItems[i].series->attachAxis(ChartItems[i].yaxis);
        ChartItems[i].areaseries->attachAxis(ChartItems[i].xaxis);
        ChartItems[i].areaseries->attachAxis(ChartItems[i].yaxis);
        ui.verticalLayout_2->addWidget(ChartItems[i].chartView);
    }

	
}

void ProgressWindow::SetTitle(const QString &title, int chart)
{
    ChartItems[chart].chart->setTitle(title);
}

ProgressWindow::~ProgressWindow()
{
}

void ProgressWindow::AppendPoint(const double& x, const double& y, int chart)
{
    if (ChartItems[chart].series->count()==0)
    {
        ChartItems[chart].yaxis->setMin(y-0.01);
        ChartItems[chart].yaxis->setMax(y+0.01);
    }
    ChartItems[chart].series->append(x, y);
    if (y<ChartItems[chart].yaxis->min())
        ChartItems[chart].yaxis->setMin(y);
    if (y>ChartItems[chart].yaxis->max())
        ChartItems[chart].yaxis->setMax(y);
    if (x<ChartItems[chart].xaxis->min())
        ChartItems[chart].xaxis->setMin(x);
    if (x>ChartItems[chart].xaxis->max())
        ChartItems[chart].xaxis->setMax(x);
    ChartItems[chart].chartView->update();

}

void ProgressWindow::ClearGraph(int chart)
{
    ChartItems[chart].series->clear();
}

void ProgressWindow::SetProgress(const double& prog)
{
	ui.progressBar->setValue(prog * 100);
    QCoreApplication::processEvents();
}

void ProgressWindow::SetProgress2(const double& prog)
{
    ui.progressBar_2->setValue(prog * 100);
    QCoreApplication::processEvents();
}

void ProgressWindow::SetLabel(const QString& label)
{
    ui.label->setText(label);
    QCoreApplication::processEvents();
}


void ProgressWindow::SetYRange(const double &y0, const double &y1, int chart)
{
    ChartItems[chart].yaxis->setMin(y0);
    ChartItems[chart].yaxis->setMax(y1);
    ChartItems[chart].chartView->update();
	
}

void ProgressWindow::SetXRange(const double &x0, const double &x1, int chart)
{
    ChartItems[chart].xaxis->setMin(x0);
    ChartItems[chart].xaxis->setMax(x1);
    ChartItems[chart].xaxis->setTitleText("Generation");
    ChartItems[chart].chartView->update();
}

void ProgressWindow::SetXAxisTitle(const QString &title, int chart)
{
    ChartItems[chart].xaxis->setTitleText(title);
}
void ProgressWindow::SetYAxisTitle(const QString &title, int chart)
{
    ChartItems[chart].yaxis->setTitleText(title);
}
