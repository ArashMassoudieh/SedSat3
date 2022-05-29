#include "plotwindow.h"
#include "ui_plotwindow.h"
#include "QComboBox"
#include "mainwindow.h"

PlotWindow::PlotWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlotWindow)
{
    ui->setupUi(this);
    combo_groups = new QComboBox(this);
    combo_sample = new QComboBox(this);
    for (int i=0; i<Data()->GroupNames().size(); i++)
    {
        combo_groups->addItem(QString::fromStdString(Data()->GroupNames()[i]));
    }

    ui->horizontalLayout->addWidget(combo_groups);
    ui->horizontalLayout->addWidget(combo_sample);
    connect(combo_groups,SIGNAL(currentIndexChanged(int)),this,SLOT(on_combo_group_changed()));
    connect(combo_sample,SIGNAL(currentIndexChanged(int)),this,SLOT(on_combo_sample_changed()));
    plot = new CustomPlotBar(this);
    ui->horizontalLayout_2->addWidget(plot);

}

PlotWindow::~PlotWindow()
{
    delete ui;
}

SourceSinkData *PlotWindow::Data()
{
    return dynamic_cast<MainWindow*>(parent())->Data();
}

void PlotWindow::on_combo_group_changed()
{
    combo_sample->clear();
    vector<string> sample_names = Data()->SampleNames(combo_groups->currentText().toStdString());
    for (int i=0; i<sample_names.size(); i++)
    {
        combo_sample->addItem(QString::fromStdString(sample_names[i]));
    }
}

void PlotWindow::on_combo_sample_changed()
{
    PlotData();
}

void PlotWindow::PlotData()
{
    // set dark background gradient:
    //QLinearGradient gradient(0, 0, 0, 400);
    //gradient.setColorAt(0, QColor(90, 90, 90));
    //gradient.setColorAt(0.38, QColor(105, 105, 105));
    //gradient.setColorAt(1, QColor(70, 70, 70));
    //plot->setBackground(QBrush(gradient));
    string Group_Selected = combo_groups->currentText().toStdString();
    string Sample_Selected = combo_sample->currentText().toStdString();
    if (!Data()->sample_set(Group_Selected))
        return;
    if (!Data()->sample_set(Group_Selected)->Profile(Sample_Selected))
        return;
    vector<double> vals = Data()->sample_set(Group_Selected)->Profile(Sample_Selected)->Vals();
    vector<string> element_names = Data()->ElementNames();
    // create empty bar chart objects:
    plot->clearGraphs();
    plot->clearPlottables();
    QCPGraph *element_concentrations = new QCPGraph(plot->xAxis, plot->yAxis);

    element_concentrations->setAntialiased(false); // gives more crisp, pixel aligned bar borders

    // set names and colors:
    element_concentrations->setName("Element Concentration");
    element_concentrations->setPen(QPen(QColor(0, 168, 140).lighter(130)));
    //element_concentrations->setBrush(QColor(0, 168, 140));
    element_concentrations->setLineStyle(QCPGraph::lsNone);
    element_concentrations->setScatterStyle(QCPScatterStyle::ssDisc);
    // prepare x axis with country labels:
    QVector<double> ticks;
    QVector<QString> labels;
    QVector<double> qvals = QVector<double>::fromStdVector(vals);
    for (int i=0; i<vals.size(); i++)
    {   ticks << i+1;
        labels << QString::fromStdString(element_names[i]);
    }
    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    textTicker->addTicks(ticks, labels);
    plot->xAxis->setTicker(textTicker);
    plot->xAxis->setTickLabelRotation(60);
    plot->xAxis->setSubTicks(false);
    plot->xAxis->setTickLength(0, 4);
    plot->xAxis->setRange(0, vals.size());
    plot->xAxis->setBasePen(QPen(Qt::black));
    plot->xAxis->setTickPen(QPen(Qt::black));
    plot->xAxis->grid()->setVisible(true);
    plot->xAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));
    plot->xAxis->setTickLabelColor(Qt::black);
    plot->xAxis->setLabelColor(Qt::black);

    // prepare y axis:
    plot->yAxis->setRange(0.01, 10000);
    plot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    plot->yAxis->setPadding(5); // a bit more space to the left border
    plot->yAxis->setLabel("Concentration");
    plot->yAxis->setBasePen(QPen(Qt::black));
    plot->yAxis->setTickPen(QPen(Qt::black));
    plot->yAxis->setSubTickPen(QPen(Qt::black));
    plot->yAxis->grid()->setSubGridVisible(true);
    plot->yAxis->setTickLabelColor(Qt::black);
    plot->yAxis->setLabelColor(Qt::black);
    plot->yAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::SolidLine));
    plot->yAxis->grid()->setSubGridPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));

    // Add data:

    element_concentrations->setData(ticks, qvals);


    // setup legend:
    plot->legend->setVisible(true);
    plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignHCenter);
    plot->legend->setBrush(QColor(255, 255, 255, 100));
    plot->legend->setBorderPen(Qt::NoPen);
    QFont legendFont = font();
    legendFont.setPointSize(10);
    plot->legend->setFont(legendFont);
    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

