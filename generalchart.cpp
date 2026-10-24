// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#include "generalchart.h"
#include "ui_generalchart.h"
#include "resultitem.h"
#include <cmath>
#include <limits>
#include <string>


GeneralChart::GeneralChart(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GeneralChart)
{
    ui->setupUi(this);
    chart = new Chart();
    chartView = new ChartView(chart);
    ui->verticalLayout->addWidget(chartView);
    connect(ui->Exporttppng,SIGNAL(clicked()),this,SLOT(on_Exporttopng()));

}

GeneralChart::~GeneralChart()
{
    delete ui;
}

bool GeneralChart::Plot(ResultItem* res)
{
    result_item = res;
    if (res->Type() == result_type::contribution)
    {

        Contribution* contributions = static_cast<Contribution*>(res->Result());
        return PlotContribution(contributions, QString::fromStdString(res->Name()));

    }
    if (res->Type() == result_type::elemental_profile_set)
    {
        
        Elemental_Profile_Set* profile_sets = static_cast<Elemental_Profile_Set*>(res->Result());
        return PlotProfileSet(profile_sets,QString::fromStdString(res->Name()));

    }

    if (res->Type() == result_type::predicted_concentration)
    {

        Elemental_Profile* profile_set = static_cast<Elemental_Profile*>(res->Result());
        return PlotPredictedConcentration(profile_set,QString::fromStdString(res->Name()));

    }

    if (res->Type() == result_type::vector)
    {
        CMBVector* profile = static_cast<CMBVector*>(res->Result());
        return PlotVector(profile, QString::fromStdString(res->Name()));
    }
    if (res->Type() == result_type::vectorset)
    {
        CMBVectorSet* profile = static_cast<CMBVectorSet*>(res->Result());
        return PlotVectorSet(profile, QString::fromStdString(res->Name()));
    }
    if (res->Type() == result_type::matrix)
    {
        CMBMatrix* matrix = static_cast<CMBMatrix*>(res->Result());
        return PlotMatrix(matrix, QString::fromStdString(res->Name()));
    }


    if (res->Type() == result_type::mlrset)
    {
        MultipleLinearRegressionSet* mlrset = static_cast<MultipleLinearRegressionSet*>(res->Result());
        return PlotRegressionSet(mlrset, QString::fromStdString(res->Name()));
    }
    if (res->Type() == result_type::timeseries_set)
    {
        CMBTimeSeriesSet* timeseriesset = static_cast<CMBTimeSeriesSet*>(res->Result());
        return PlotTimeSeriesSet(timeseriesset, QString::fromStdString(res->Name()),QString::fromStdString(res->XAxisTitle()),QString::fromStdString(res->YAxisTitle()));
    }
    if (res->Type() == result_type::rangeset || res->Type() == result_type::rangeset_with_observed)
    {
        RangeSet* rangeset = static_cast<RangeSet*>(res->Result());
        return PlotRangeSet(rangeset, QString::fromStdString(res->Name()),QString::fromStdString(res->XAxisTitle()),QString::fromStdString(res->YAxisTitle()));
    }

    if (res->Type() == result_type::timeseries_set_first_symbol)
    {
        CMBTimeSeriesSet* timeseriesset = static_cast<CMBTimeSeriesSet*>(res->Result());
        return PlotTimeSeriesSet_M(timeseriesset, QString::fromStdString(res->Name()),QString::fromStdString(res->XAxisTitle()),QString::fromStdString(res->YAxisTitle()));
    }
    if (res->Type() == result_type::timeseries_set_first_symbol)
    {
        CMBTimeSeriesSet* timeseriesset = static_cast<CMBTimeSeriesSet*>(res->Result());
        return PlotTimeSeriesSet_M(timeseriesset, QString::fromStdString(res->Name()),QString::fromStdString(res->XAxisTitle()),QString::fromStdString(res->YAxisTitle()));
    }
    if (res->Type() == result_type::stacked_bar_chart)
    {
        CMBTimeSeriesSet* timeseriesset = static_cast<CMBTimeSeriesSet*>(res->Result());
        return PlotTimeSeriesSet_Stacked(timeseriesset, QString::fromStdString(res->Name()),QString::fromStdString(res->XAxisTitle()),QString::fromStdString(res->YAxisTitle()));
    }

    if (res->Type() == result_type::mcmc_samples)
    {
        CMBTimeSeriesSet* timeseriesset = static_cast<CMBTimeSeriesSet*>(res->Result());
        return InitializeMCMCSamples(timeseriesset, QString::fromStdString(res->Name()));
    }
    if (res->Type() == result_type::distribution || res->Type() == result_type::distribution_with_observed)
    {
        CMBTimeSeriesSet* timeseriesset = static_cast<CMBTimeSeriesSet*>(res->Result());
        return InitializeDistributions(timeseriesset, QString::fromStdString(res->Name()));
    }
    if (res->Type() == result_type::matrix1vs1)
    {
        CMBMatrix* matrix = static_cast<CMBMatrix*>(res->Result());
        return PlotScatter(matrix);
    }
    if (res->Type() == result_type::vectorset_groups)
    {
        CMBVectorSet* vector_set = static_cast<CMBVectorSet*>(res->Result());
        return PlotScatter(vector_set);
    }

    if (res->Type() == result_type::dfa_vectorsetset)
    {
        CMBVectorSetSet* vector_set = static_cast<CMBVectorSetSet*>(res->Result());
        return PlotVectorSetSet(vector_set,QString::fromStdString(res->Name()));
    }

    chartView->update();
    return false; 
}

// Rounds down to a multiple of rounding_value. It used to round to the
// nearest multiple, so an axis could start above the smallest value and cut
// points off the plot.
double roundDown(double a, double rounding_value) {
    return std::floor(a/rounding_value)*rounding_value;
}

// Color-blind-safe palette (Okabe and Ito), blue first; yellow last, as it
// shows least on white
static const QColor series_palette[] = {
    QColor(0, 114, 178), QColor(230, 159, 0), QColor(0, 158, 115), QColor(213, 94, 0),
    QColor(204, 121, 167), QColor(86, 180, 233), QColor(0, 0, 0), QColor(240, 228, 66)};

// The same series name keeps the same color in this window (e.g. a source
// group across the DFA pairs)
QColor GeneralChart::SeriesColor(const QString& name)
{
    auto it = series_colors.find(name);
    if (it == series_colors.end())
        it = series_colors.insert(name, series_palette[series_colors.size() % std::size(series_palette)]);
    return it.value();
}

// Axis titles for plots with one category per constituent. The default x
// title of a result is "Sample" and the default y title "Value"; neither
// describes these plots.
QString GeneralChart::ConstituentAxisTitle() const
{
    const QString title = QString::fromStdString(result_item->XAxisTitle());
    return (title.isEmpty() || title == "Sample") ? QString("Constituent") : title;
}

QString GeneralChart::ValueAxisTitle(const QString& fallback) const
{
    const QString title = QString::fromStdString(result_item->YAxisTitle());
    if (!title.isEmpty() && title != "Value")
        return title;
    return fallback.isEmpty() ? QString::fromStdString(result_item->Name()) : fallback;
}

// Sets a log axis to whole decades around [low, high], starting one decade
// below the smallest value so its bar or point is drawn (at the bottom of the
// axis it had no height). Qt Charts draws no axis at all over a very wide
// range (p-values reach 1e-40), so beyond ten decades the labels go every
// n-th decade.
static void SetLogRange(QLogValueAxis* axis, double low, double high)
{
    if (!(low > 0) || !(high > 0) || !std::isfinite(low) || !std::isfinite(high))
        return;
    int low_exponent = int(std::ceil(std::log10(low))) - 1;
    int high_exponent = int(std::floor(std::log10(high))) + 1;
    const int step = std::max(1, int(std::ceil((high_exponent - low_exponent) / 10.0)));
    low_exponent = int(std::floor(double(low_exponent) / step)) * step;
    high_exponent = int(std::ceil(double(high_exponent) / step)) * step;
    axis->setBase(std::pow(10.0, step));
    axis->setRange(std::pow(10.0, low_exponent), std::pow(10.0, high_exponent));
    axis->setMinorTickCount(step == 1 ? 8 : 0);
}

// Widens [low, high] by a fraction of its span so points at the extremes are
// drawn inside the plot, not on its edge
static void PadRange(double& low, double& high, double fraction = 0.05)
{
    double span = high - low;
    if (!(span > 0))
        span = std::max(std::fabs(high), 1.0);
    low -= span * fraction;
    high += span * fraction;
}

bool GeneralChart::PlotVectorSetSet(CMBVectorSetSet *profile, const QString &/*title*/)
{
    source1_combo = new QComboBox();
    source2_combo = new QComboBox();
    QLabel *source1_label = new QLabel();
    QLabel *source2_label = new QLabel();
    source1_label->setText("Source group I:");
    source2_label->setText("Source group II:");

    connect(source1_combo, SIGNAL(currentIndexChanged(int)),this, SLOT(onDFAPairChanged(int)));
    connect(source2_combo, SIGNAL(currentIndexChanged(int)),this, SLOT(onDFAPairChanged(int)));

    for (map<std::string,CMBVectorSet>::iterator it = profile->begin(); it!=profile->end(); it++ )
    {
        source1_combo->addItem(QString::fromStdString(it->first));
        source2_combo->addItem(QString::fromStdString(it->first));
    }
    ui->horizontalLayout->addWidget(source1_label);
    ui->horizontalLayout->addWidget(source1_combo);
    ui->horizontalLayout->addWidget(source2_label);
    ui->horizontalLayout->addWidget(source2_combo);
    // Fix each group's color for every pair, and start on two different
    // groups (both used to start on the first, plotting it against itself)
    for (int i = 0; i < source1_combo->count(); i++)
        SeriesColor(source1_combo->itemText(i));
    if (source2_combo->count() > 1)
    {
        source2_combo->blockSignals(true);
        source2_combo->setCurrentIndex(1);
        source2_combo->blockSignals(false);
    }
    onDFAPairChanged(source1_combo->currentIndex());
    return true;
}

bool GeneralChart::PlotVectorSet(CMBVectorSet *profile, const QString &/*title*/)
{
    element_combo = new QComboBox();
    QLabel *element_label = new QLabel();
    element_label->setText("Source group pair:");

    connect(element_combo, SIGNAL(currentIndexChanged(int)),this, SLOT(onPairChanged(int)));

    for (map<std::string,CMBVector>::iterator it = profile->begin(); it!=profile->end(); it++ )
    {
        element_combo->addItem(QString::fromStdString(it->first));
    }


    ui->horizontalLayout->addWidget(element_label);
    ui->horizontalLayout->addWidget(element_combo);
    onPairChanged(element_combo->currentIndex());

    return true;
}

bool GeneralChart::PlotVector(CMBVector *profile, const QString &title)
{
    QStringList categories;

    QBarCategoryAxis *axisX = new QBarCategoryAxis();

    vector<string> element_names = profile->Labels();
    for (unsigned int i = 0; i < element_names.size(); i++)
        categories << QString::fromStdString(element_names[i]);

    axisX->append(categories);
    axisX->setTitleText(ConstituentAxisTitle());
    QLogValueAxis* axisYLog = nullptr;
    QValueAxis* axisYNormal = nullptr;
    bool _log = (result_item->YAxisMode()==yaxis_mode::log?true:false);
    double profile_min = NAN;
    if (result_item->AbsValue())
        profile_min = profile->abs().min();
    else
        profile_min = profile->min();
    // Qt's log axis treats values below about 1e-12 as zero and then draws
    // no axis; such values (p-values reach 1e-40) are shown as log10 on a
    // linear axis instead
    const bool plot_log10 = _log && profile_min > 0 && profile_min < 1e-12;
    if (plot_log10)
        _log = false;
    if (profile_min>0 && _log)
    {
        axisYLog = new QLogValueAxis();
        if (result_item->AbsValue())
            SetLogRange(axisYLog, profile->abs().min(), profile->abs().max());
        else
            SetLogRange(axisYLog, profile->min(), profile->max());
        axisYLog->setLabelFormat("%g");
        axisYLog->setTitleText(ValueAxisTitle());
        chart->addAxis(axisYLog, Qt::AlignLeft);
        _log = true;
    }
    else
    {
        axisYNormal = new QValueAxis();
        if (plot_log10)
        {
            const double low = std::log10(profile_min);
            const double high = std::log10(result_item->AbsValue() ? profile->abs().max() : profile->max());
            axisYNormal->setRange(std::floor(low) - 1, std::ceil(high));
        }
        else if (result_item->AbsValue())
            axisYNormal->setRange(roundDown(profile->abs().min()*10.0)/10, roundDown((profile->abs().max()+0.1)*10.0)/10);
        else
            axisYNormal->setRange(roundDown(profile->min()*10.0)/10, roundDown((profile->max()+0.1)*10.0)/10);
        axisYNormal->setLabelFormat("%g");
        axisYNormal->setMinorTickCount(5);
        // A grid line through zero, so positive and negative bars (e.g. the
        // Box-Cox parameters) read against it
        if (axisYNormal->min() < 0 && axisYNormal->max() > 0)
        {
            axisYNormal->setTickType(QValueAxis::TicksDynamic);
            axisYNormal->setTickAnchor(0);
            axisYNormal->setTickInterval(std::pow(10.0, std::floor(std::log10(axisYNormal->max() - axisYNormal->min())))/2.0);
        }
        axisYNormal->setTitleText(plot_log10 ? "log10(" + ValueAxisTitle() + ")" : ValueAxisTitle());
        chart->addAxis(axisYNormal, Qt::AlignLeft);
        _log = false;
    }
    axisX->setLabelsAngle(-90);
    chart->addAxis(axisX, Qt::AlignBottom);


    //QLineSeries* series = new QLineSeries();
    QBarSeries* series = new QBarSeries();

    chart->addSeries(series);
    series->setName(title);
    series->attachAxis(axisX);
    if (_log)
        series->attachAxis(axisYLog);
    else
        series->attachAxis(axisYNormal);

    double counter = 0.5;
    QBarSet *barset = new QBarSet(QString::fromStdString(result_item->Name()));
    QPen pen;
    pen.setWidth(2);
    barset->setPen(pen);

    for (int i=0; i<profile->num; i++)
    {
        const double value = result_item->AbsValue() ? fabs(profile->at(i)) : profile->at(i);
        barset->append(plot_log10 ? std::log10(value) : value);
        counter++;
    }
    series->append(barset);
    return true;

}


bool GeneralChart::PlotMatrix(CMBMatrix *matrix, const QString &/*title*/)
{
    QStringList categories;

    QBarCategoryAxis *axisX = new QBarCategoryAxis();

    vector<string> element_names = matrix->ColumnLabels();
    for (unsigned int i = 0; i < element_names.size(); i++)
        categories << QString::fromStdString(element_names[i]);

    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);

    QLogValueAxis* axisYLog = nullptr;
    QValueAxis* axisYNormal = nullptr;
    bool _log = (result_item->YAxisMode()==yaxis_mode::log?true:false);
    double matrix_min = NAN;
    if (result_item->AbsValue())
        matrix_min = matrix->abs().min();
    else
        matrix_min = matrix->min();


    if (matrix_min>0 && _log)
    {
        axisYLog = new QLogValueAxis();
        SetLogRange(axisYLog, matrix_min, matrix->max());
        axisYLog->setLabelFormat("%g");
        chart->addAxis(axisYLog, Qt::AlignLeft);
        _log = true;
    }
    else
    {
        axisYNormal = new QValueAxis();
        axisYNormal->setRange(roundDown(matrix_min*10.0)/10, roundDown((matrix->max()+0.1)*10.0)/10);
        axisYNormal->setLabelFormat("%f");
        axisYNormal->setMinorTickCount(5);
        chart->addAxis(axisYNormal, Qt::AlignLeft);
        _log= false;
    }

    chart->addAxis(axisX, Qt::AlignBottom);


    for (int i=0; i<matrix->getnumrows(); i++)
    {   QBarSeries* series = new QBarSeries();

        chart->addSeries(series);
        series->setName(QString::fromStdString(matrix->RowLabel(i)));
        series->attachAxis(axisX);
        if (_log)
            series->attachAxis(axisYLog);
        else
            series->attachAxis(axisYNormal);

        double counter = 0.5;
        QBarSet *barset = new QBarSet(QString::fromStdString(matrix->RowLabel(i)));
        barset->setColor(SeriesColor(barset->label()));
        QPen pen;
        pen.setWidth(2);
        barset->setPen(pen);

        for (int j=0; j<matrix->getnumcols(); j++)
        {
            if (result_item->AbsValue())
                barset->append(fabs(matrix->matr[i][j]));
            else
                barset->append(matrix->matr[i][j]);
            counter++;
        }
        series->append(barset);
    }
    return true;


}


bool GeneralChart::PlotProfileSet(Elemental_Profile_Set *profile_sets, const QString &/*title*/)
{
    QCategoryAxis* axisX = new QCategoryAxis();

    axisX->setRange(0, profile_sets->GetElementNames().size());
    vector<string> element_names = profile_sets->GetElementNames();
    for (int i=0; i<element_names.size(); i++)
        axisX->append(QString::fromStdString(element_names[i]),double(i+1));


    axisX->setTitleText(ConstituentAxisTitle());

    QLogValueAxis* axisYLog = nullptr;
    QValueAxis* axisYNormal = nullptr;
    bool _log = (result_item->YAxisMode()==yaxis_mode::log?true:false);
    if (profile_sets->GetMinimum()>0 && _log)
    {
        axisYLog = new QLogValueAxis();
        SetLogRange(axisYLog, profile_sets->GetMinimum(), profile_sets->GetMaximum());
        axisYLog->setLabelFormat("%g");
        axisYLog->setTitleText(ValueAxisTitle("Value"));
        chart->addAxis(axisYLog, Qt::AlignLeft);
        _log = true;
    }
    else
    {
        axisYNormal = new QValueAxis();
        double low = profile_sets->GetMinimum();
        double high = result_item->YLimit(_range::high)==0 ? profile_sets->GetMaximum() : result_item->YLimit(_range::high);
        PadRange(low, high);
        axisYNormal->setRange(low, high);
        axisYNormal->setLabelFormat("%g");
        axisYNormal->setMinorTickCount(5);
        axisYNormal->setTitleText(ValueAxisTitle("Value"));
        chart->addAxis(axisYNormal, Qt::AlignLeft);
        // A linear axis was built, so the series must attach to it. PlotVector
        // already does this; without it, data whose minimum is not positive
        // takes the log branch below and attaches to an axis never created.
        _log = false;
    }

    chart->addAxis(axisX, Qt::AlignBottom);


    for (map<string,Elemental_Profile>::iterator it = profile_sets->begin(); it!=profile_sets->end(); it++)
    {
        QScatterSeries* series = new QScatterSeries();
        chart->addSeries(series);
        series->setName(QString::fromStdString(it->first));
        series->setColor(SeriesColor(series->name()));
        series->attachAxis(axisX);
        if (_log)
            series->attachAxis(axisYLog);
        else
            series->attachAxis(axisYNormal);
        series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
        // Each later series is drawn smaller, on top, so a point that matches
        // an earlier one (observed on modeled) does not hide it
        series->setMarkerSize(std::max(7.0, 16.0 - 5.0 * double(chart->series().size() - 1)));
        double counter=0.5;
        for (map<string, double>::iterator datapoint = it->second.begin(); datapoint != it->second.end(); datapoint++)
        {
            series->append(counter,datapoint->second);
            counter++;
        }

    }
    return true;

}
bool GeneralChart::PlotContribution(Contribution* contributions, const QString &title)
{
    QPieSeries* series = new QPieSeries();
    QStringList labels;
    for (map<string, double>::iterator it = contributions->begin(); it != contributions->end(); it++)
    {
        series->append(QString::fromStdString(it->first), it->second * 100);
        labels.append(QString::number(it->second*100)+"%");
    }

    chart->addSeries(series);
    for(auto slice : series->slices())
    {   slice->setColor(SeriesColor(slice->label()));
        slice->setLabel(QString("%1%").arg(100*slice->percentage(), 0, 'f', 1));
        slice->setLabelVisible();

    }

    int i=0;
    for (map<string, double>::iterator it = contributions->begin(); it != contributions->end(); it++)
    {
        chart->legend()->markers(series)[i]->setLabel(QString::fromStdString(it->first));
        i++;
    }

    series->setLabelsPosition(QPieSlice::LabelOutside);
    chart->setTitle(title);

    chartView->setRenderHint(QPainter::Antialiasing);
    return true; 
}
bool GeneralChart::PlotPredictedConcentration(Elemental_Profile* profile_set, const QString &title)
{
    QCategoryAxis* axisX = new QCategoryAxis();
    // qDebug() << profile_set->GetElementNames().size();
    axisX->setRange(0, profile_set->size()*10);
    vector<string> element_names = profile_set->GetElementNames();
    for (int i = 0; i < element_names.size(); i++)
        axisX->append(QString::fromStdString(element_names[i]), double(i + 1)*10);


    QLogValueAxis* axisYLog = nullptr;
    QValueAxis* axisYNormal = nullptr;
    axisX->setTitleText(ConstituentAxisTitle());
    bool _log = (result_item->YAxisMode()==yaxis_mode::log?true:false);
    if (profile_set->GetMinimum()>0 && _log)
    {
        axisYLog = new QLogValueAxis();
        SetLogRange(axisYLog, profile_set->GetMinimum(), profile_set->GetMaximum());
        axisYLog->setLabelFormat("%g");
        axisYLog->setTitleText(ValueAxisTitle("Value"));
        chart->addAxis(axisYLog, Qt::AlignLeft);
        _log = true;
    }
    else
    {
        axisYNormal = new QValueAxis();
        double low = profile_set->GetMinimum();
        double high = result_item->YLimit(_range::high)==0 ? profile_set->GetMaximum() : result_item->YLimit(_range::high);
        PadRange(low, high);
        axisYNormal->setRange(low, high);
        axisYNormal->setLabelFormat("%g");
        axisYNormal->setMinorTickCount(5);
        axisYNormal->setTitleText(ValueAxisTitle("Value"));
        chart->addAxis(axisYNormal, Qt::AlignLeft);
        // A linear axis was built, so the series must attach to it. PlotVector
        // already does this; without it, data whose minimum is not positive
        // takes the log branch below and attaches to an axis never created.
        _log = false;
    }

    chart->addAxis(axisX, Qt::AlignBottom);


    //QLineSeries* series = new QLineSeries();
    QScatterSeries* series = new QScatterSeries();
    chart->addSeries(series);
    series->setName(title);
    series->attachAxis(axisX);
    if (_log)
        series->attachAxis(axisYLog);
    else
        series->attachAxis(axisYNormal);
    series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    series->setMarkerSize(15.0);
    double counter = 0.5;
    for (map<string, double>::iterator datapoint = profile_set->begin(); datapoint != profile_set->end(); datapoint++)
    {
        series->append(counter*10, datapoint->second);
        counter++;
    }
    return true;


}
bool GeneralChart::PlotRegressionSet(MultipleLinearRegressionSet *regressionset, const QString &/*title*/)
{
    element_combo = new QComboBox();
    QLabel *element_label = new QLabel();
    element_label->setText("Constituent:");
    independent_combo = new QComboBox();

    connect(element_combo, SIGNAL(currentIndexChanged(int)),this, SLOT(onElementChanged(int)));
    connect(independent_combo, SIGNAL(currentIndexChanged(int)),this, SLOT(onIndependentChanged(int)));
    for (map<std::string,MultipleLinearRegression>::iterator it = regressionset->begin(); it!=regressionset->end(); it++ )
    {
        element_combo->addItem(QString::fromStdString(it->first));
    }

    QLabel *independent_label = new QLabel();
    independent_label->setText("Independent Variable:");
    ui->horizontalLayout->addWidget(element_label);
    ui->horizontalLayout->addWidget(element_combo);
    ui->horizontalLayout->addWidget(independent_label);
    ui->horizontalLayout->addWidget(independent_combo);
    onElementChanged(element_combo->currentIndex());
    onIndependentChanged(independent_combo->currentIndex());
    return true;
}

bool GeneralChart:: InitializeMCMCSamples(CMBTimeSeriesSet *mcmcsamples, const QString &/*title*/)
{
    element_combo = new QComboBox();
    QLabel *element_label = new QLabel();
    element_label->setText("Variable:");
    ui->horizontalLayout->addWidget(element_label);
    ui->horizontalLayout->addWidget(element_combo);

    for (int i = 0; i<mcmcsamples->size(); i++ )
    {
        element_combo->addItem(QString::fromStdString(mcmcsamples->getSeriesName(i)));
    }
    connect(element_combo, SIGNAL(currentIndexChanged(int)),this, SLOT(onMCMCVariableChanged(int)));


    onMCMCVariableChanged(element_combo->currentIndex());

    return true;
}

bool GeneralChart::InitializeDistributions(CMBTimeSeriesSet *distributions, const QString &/*title*/)
{
    element_combo = new QComboBox();
    QLabel *element_label = new QLabel();
    element_label->setText("Variable:");
    ui->horizontalLayout->addWidget(element_label);
    ui->horizontalLayout->addWidget(element_combo);

    for (int i = 0; i<distributions->size(); i++ )
    {
        element_combo->addItem(QString::fromStdString(distributions->getSeriesName(i)));
    }
    connect(element_combo, SIGNAL(currentIndexChanged(int)),this, SLOT(onDistributionsVariableChanged(int)));


    onDistributionsVariableChanged(element_combo->currentIndex());

    return true;
}


void GeneralChart::onElementChanged(int i_element)
{
    MultipleLinearRegressionSet* mlrset = static_cast<MultipleLinearRegressionSet*>(result_item->Result());
    independent_combo->clear();
    QString constituent = element_combo->itemText(i_element);
    for (unsigned int i=0; i<mlrset->at(constituent.toStdString()).GetIndependentVariableNames().size(); i++)
    {
        independent_combo->addItem(QString::fromStdString(mlrset->at(constituent.toStdString()).GetIndependentVariableNames()[i]));
    }
    if (!independent_combo->currentText().isEmpty() && !element_combo->currentText().isEmpty())
    {
        PlotRegression(&mlrset->at(constituent.toStdString()),independent_combo->currentText());
    }

}

void GeneralChart::onPairChanged(int pair_id)
{
    chart->removeAllSeries();
    for (int i=0; i<chart->axes().size(); i++)
        chart->removeAxis(chart->axes()[i]);

    for (int i=0; i<chart->axes(Qt::Vertical).size(); i++)
    {
        chart->removeAxis(chart->axes(Qt::Vertical)[i]);
    }

    for (int i=0; i<chart->axes(Qt::Horizontal).size(); i++)
    {
        chart->removeAxis(chart->axes(Qt::Horizontal)[i]);
    }
    CMBVectorSet* vectorset = static_cast<CMBVectorSet*>(result_item->Result());
    QString pair = element_combo->itemText(pair_id);
    PlotVector(&vectorset->at(pair.toStdString()),"DFA S value between '" + pair + "'");

}

void GeneralChart::onDFAPairChanged(int /*pair_id*/)
{
    chart->removeAllSeries();
    for (int i=0; i<chart->axes().size(); i++)
        chart->removeAxis(chart->axes()[i]);

    for (int i=0; i<chart->axes(Qt::Vertical).size(); i++)
    {
        chart->removeAxis(chart->axes(Qt::Vertical)[i]);
    }

    for (int i=0; i<chart->axes(Qt::Horizontal).size(); i++)
    {
        chart->removeAxis(chart->axes(Qt::Horizontal)[i]);
    }
    CMBVectorSetSet* vectorset = static_cast<CMBVectorSetSet*>(result_item->Result());
    // qDebug()<<source1_combo->currentText();
    // qDebug()<<source2_combo->currentText();
    if (source1_combo->currentText()!="" && source2_combo->currentText()!="")
        PlotScatter(&vectorset->at(source1_combo->currentText().toStdString()),&vectorset->at(source2_combo->currentText().toStdString()), "Discriminant score, " + source1_combo->currentText() + " vs. the rest", "Discriminant score, " + source2_combo->currentText() + " vs. the rest");
}

void GeneralChart::onMCMCVariableChanged(int i)
{
    CMBTimeSeriesSet * samplesset = static_cast<CMBTimeSeriesSet*>(result_item->Result());
    QString variable = element_combo->itemText(i);
    PlotMCMCSamples(&samplesset->operator[](variable.toStdString()),variable);
}

void GeneralChart::onDistributionsVariableChanged(int i)
{
    CMBTimeSeriesSet * samplesset = static_cast<CMBTimeSeriesSet*>(result_item->Result());
    QString variable = element_combo->itemText(i);
    if (samplesset->Contains(variable.toStdString()))
        PlotDistribution(&samplesset->operator[](variable.toStdString()),variable);
}
void GeneralChart::onIndependentChanged(int /*i_constituent*/)
{
    MultipleLinearRegressionSet* mlrset = static_cast<MultipleLinearRegressionSet*>(result_item->Result());
    if (!independent_combo->currentText().isEmpty() && !element_combo->currentText().isEmpty())
    {
        PlotRegression(&mlrset->at(element_combo->currentText().toStdString()),independent_combo->currentText());
    }

}

bool GeneralChart::PlotRegression(MultipleLinearRegression *mlr,const QString& independent_var)
{

    for (int i=0; i<chart->axes(Qt::Horizontal).size(); i++)
    {
        // qDebug()<<chart->axes(Qt::Horizontal)[i]->objectName();
    }

    for (int i=0; i<chart->axes(Qt::Vertical).size(); i++)
    {
        // qDebug()<<chart->axes(Qt::Vertical)[i]->objectName();
    }

    chart->removeAllSeries();
    for (int i=0; i<chart->axes().size(); i++)
    {
        chart->removeAxis(chart->axes()[i]);
        //delete chart->axes()[i];
    }

    // qDebug()<<"After:";
    for (int i=0; i<chart->axes(Qt::Horizontal).size(); i++)
    {
        // qDebug()<<chart->axes(Qt::Horizontal)[i]->objectName();
        chart->removeAxis(chart->axes(Qt::Horizontal)[i]);
    }

    for (int i=0; i<chart->axes(Qt::Vertical).size(); i++)
    {
        // qDebug()<<chart->axes(Qt::Vertical)[i]->objectName();
        chart->removeAxis(chart->axes(Qt::Vertical)[i]);
    }
    chart->axes().clear();
    QValueAxis* axisX = new QValueAxis();
    QValueAxis* axisYNormal = new QValueAxis();
    axisX->setObjectName("axisX");
    axisYNormal->setObjectName("axisY");

    QLineSeries *lineseries = new QLineSeries();
    double x_min_val = CVector(mlr->IndependentData(independent_var.toStdString())).min();
    double x_max_val = CVector(mlr->IndependentData(independent_var.toStdString())).max();
    double y_min_val = NAN, y_max_val = NAN;

    if (mlr->Equation()==regression_form::linear)
    {   y_min_val = mlr->CoefficientsIntercept()[0];
        y_max_val = mlr->CoefficientsIntercept()[0];
    }
    else
    {
        y_min_val = exp(mlr->CoefficientsIntercept()[0]);
        y_max_val = exp(mlr->CoefficientsIntercept()[0]);
    }
    for (int i=0; i<mlr->GetIndependentVariableNames().size(); i++)
    {
        if (mlr->GetIndependentVariableNames()[i]!=independent_var.toStdString())
        {
            if (mlr->Equation()==regression_form::linear)
            {   y_min_val += mlr->MeanIndependentVar(i)*mlr->CoefficientsIntercept()[i+1];
                y_max_val += mlr->MeanIndependentVar(i)*mlr->CoefficientsIntercept()[i+1];
            }
            else
            {
                y_min_val *= exp(log(mlr->GeoMeanIndependentVar(i))*mlr->CoefficientsIntercept()[i+1]);
                y_max_val *= exp(log(mlr->GeoMeanIndependentVar(i))*mlr->CoefficientsIntercept()[i+1]);
            }
        }
        else
        {
            if (mlr->Equation()==regression_form::linear)
            {   y_min_val += x_min_val*mlr->CoefficientsIntercept()[i+1];
                y_max_val += x_max_val*mlr->CoefficientsIntercept()[i+1];
            }
            else
            {
                y_min_val *= exp(log(x_min_val)*mlr->CoefficientsIntercept()[i+1]);
                y_max_val *= exp(log(x_max_val)*mlr->CoefficientsIntercept()[i+1]);
            }
        }
    }

    double x_low = CVector(mlr->IndependentData(independent_var.toStdString())).min();
    double x_high = CVector(mlr->IndependentData(independent_var.toStdString())).max();
    PadRange(x_low, x_high);
    axisX->setRange(x_low, x_high);
    axisX->setTitleText(independent_var);

    //axisYNormal->setLabelFormat("%f");
    //axisYNormal->setMinorTickCount(5);
    axisYNormal->setTitleText(QString::fromStdString("Measured " + mlr->DependentVariableName()));


    QScatterSeries* series = new QScatterSeries();

    series->setName(QString::fromStdString(mlr->DependentVariableName()));
    series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    series->setMarkerSize(15.0);
    // qDebug()<<independent_var;
    for (unsigned int i=0; i<mlr->IndependentData(independent_var.toStdString()).size(); i++)
    {
        // qDebug()<<mlr->DependentData()[i] << ","<< mlr->IndependentData(independent_var.toStdString())[i];
        series->append(mlr->IndependentData(independent_var.toStdString())[i],mlr->DependentData()[i]);
    }

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisYNormal, Qt::AlignLeft);
    chart->addSeries(series);
    series->attachAxis(axisX);
    series->attachAxis(axisYNormal);

    if (mlr->Equation()==regression_form::linear)
    {
        lineseries->append(x_min_val,y_min_val);
        lineseries->append(x_max_val,y_max_val);
    }
    else
    {
        for (double x = x_min_val; x<=x_max_val; x+=(x_max_val-x_min_val)/20.0)
        {
            double y = exp(mlr->CoefficientsIntercept()[0]);
            for (unsigned int i=0; i<mlr->GetIndependentVariableNames().size(); i++)
                if (mlr->GetIndependentVariableNames()[i]!=independent_var.toStdString())
                    y *= exp(log(mlr->GeoMeanIndependentVar(i))*mlr->CoefficientsIntercept()[i+1]);
                else
                    y *= exp(log(x)*mlr->CoefficientsIntercept()[i+1]);
            lineseries->append(x,y);
        }
    }
    chart->addSeries(lineseries);
    lineseries->attachAxis(axisX);
    lineseries->attachAxis(axisYNormal);
    lineseries->setName("Regression");
    double y_low = std::min(std::min(CVector(mlr->DependentData()).min(),y_min_val),y_max_val);
    double y_high = std::max(std::max(CVector(mlr->DependentData()).max(),y_max_val),y_min_val);
    PadRange(y_low, y_high);
    axisYNormal->setRange(y_low, y_high);

    return true;
}

bool GeneralChart::PlotScatter(CMBMatrix *matrix)
{

    QValueAxis* axisX = new QValueAxis();
    QValueAxis* axisYNormal = new QValueAxis();
    axisX->setObjectName("axisX");
    axisYNormal->setObjectName("axisY");

    CMBVector vec1 = matrix->GetColumn(matrix->ColumnLabel(0));
    CMBVector vec2 = matrix->GetColumn(matrix->ColumnLabel(1));

    double x_min_val = matrix->GetColumn(matrix->ColumnLabel(0)).min();
    double x_max_val = matrix->GetColumn(matrix->ColumnLabel(0)).max();
    double y_min_val = matrix->GetColumn(matrix->ColumnLabel(1)).min();
    double y_max_val = matrix->GetColumn(matrix->ColumnLabel(1)).max();


    axisX->setRange(x_min_val-(x_max_val-x_min_val)*0.05,x_max_val+(x_max_val-x_min_val)*0.05);
    axisX->setTitleText(QString::fromStdString(matrix->ColumnLabel(0)));
    axisYNormal->setRange(y_min_val-(y_max_val-y_min_val)*0.05,y_max_val+(y_max_val-y_min_val)*0.05);
    axisYNormal->setTitleText(QString::fromStdString(matrix->ColumnLabel(1)));

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisYNormal, Qt::AlignLeft);

    QStringList rowcategories = matrix->RowLabelCategories();
    for (int i=0; i<rowcategories.size();i++)
    {
        QScatterSeries* series = new QScatterSeries();
        // qDebug()<<rowcategories[i];
        for (int j=0; j<matrix->getnumrows(); j++)
            if (QString::fromStdString(matrix->RowLabel(j))==rowcategories[i])
                series->append(matrix->matr[j][0],matrix->matr[j][1]);

        QPen pen = series->pen();
        pen.setWidth(2);
        pen.setBrush(SeriesColor(rowcategories[i]));
        series->setColor(SeriesColor(rowcategories[i]));
        series->setPen(pen);

        series->setName(rowcategories[i]);
        series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
        series->setMarkerSize(15.0);
        chart->addSeries(series);
        series->attachAxis(axisX);
        series->attachAxis(axisYNormal);

    }

    return true;
}

bool GeneralChart::PlotScatter(CMBVectorSet *vectorset)
{

    QCategoryAxis* axisX = new QCategoryAxis();
    QValueAxis* axisYNormal = new QValueAxis();
    axisX->setObjectName("axisX");
    axisYNormal->setObjectName("axisY");

    double x_min_val = 0.5;
    double x_max_val = vectorset->size()+0.5;
    double y_min_val = vectorset->min();
    double y_max_val = vectorset->max();


    axisX->setRange(x_min_val,x_max_val);
    const QString x_title = QString::fromStdString(result_item->XAxisTitle());
    axisX->setTitleText((x_title.isEmpty() || x_title == "Sample") ? QString("Group") : x_title);
    axisYNormal->setRange(y_min_val-(y_max_val-y_min_val)*0.05,y_max_val+(y_max_val-y_min_val)*0.05);
    axisYNormal->setTitleText(ValueAxisTitle("Discriminant score"));

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisYNormal, Qt::AlignLeft);

    QStringList categories;
    int counter = 0;
    for (map<string, CMBVector>::iterator vec = vectorset->begin(); vec!=vectorset->end(); vec++)
    {
        QScatterSeries* series = new QScatterSeries();

        for (int j=0; j<vec->second.num; j++)
            series->append(counter+1,vec->second[j]);

        counter++;
        QPen pen = series->pen();
        pen.setWidth(2);
        pen.setBrush(SeriesColor(QString::fromStdString(vec->first)));
        series->setColor(SeriesColor(QString::fromStdString(vec->first)));
        series->setPen(pen);

        series->setName(QString::fromStdString(vec->first));
        series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
        series->setMarkerSize(15.0);
        chart->addSeries(series);
        series->attachAxis(axisX);
        series->attachAxis(axisYNormal);
        axisX->append(QString::fromStdString(vec->first), counter+0.5);
    }

    return true;
}


bool GeneralChart::PlotScatter(CMBVectorSet *vectorset1, CMBVectorSet *vectorset2, const QString &xaxisTitle, const QString &yaxisTitle)
{

    QValueAxis* axisX = new QValueAxis();
    QValueAxis* axisYNormal = new QValueAxis();
    axisX->setObjectName("axisX");
    axisYNormal->setObjectName("axisY");

    double x_min_val = vectorset1->min();
    double x_max_val = vectorset1->max();
    double y_min_val = vectorset2->min();
    double y_max_val = vectorset2->max();


    axisX->setRange(x_min_val-(x_max_val-x_min_val)*0.05,x_max_val+(x_max_val-x_min_val)*0.05);
    axisX->setTitleText(xaxisTitle);
    axisYNormal->setRange(y_min_val-(y_max_val-y_min_val)*0.05,y_max_val+(y_max_val-y_min_val)*0.05);
    axisYNormal->setTitleText(yaxisTitle);

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisYNormal, Qt::AlignLeft);

    QStringList categories;
    int counter = 0;


    for (map<string, CMBVector>::iterator vec = vectorset1->begin(); vec!=vectorset1->end(); vec++)
    {
        QScatterSeries* series = new QScatterSeries();

        TimeSeriesSet<double> vals(2);
        for (int j=0; j<vec->second.num; j++)
        {
            series->append(vec->second[j],vectorset2->at(vec->first)[j]);
            vals[0].append(j,vec->second[j]);
            vals[1].append(j,vectorset2->at(vec->first)[j]);
        }

        double mean1 = vals[0].mean();
        double mean2 = vals[1].mean();
        double std1 = vals[0].stddev();
        double std2 = vals[1].stddev();
        double Cov = Covariance(vals[0],vals[1]);

        QColor color = SeriesColor(QString::fromStdString(vec->first));
        counter++;


        series->setName(QString::fromStdString(vec->first));
        series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
        series->setMarkerSize(15.0);
        series->setColor(color);
        QPen pen = series->pen();
        pen.setColor(color);
        pen.setWidth(2);
        pen.setBrush(color);
        series->setPen(pen);

        chart->addSeries(series);
        series->attachAxis(axisX);
        series->attachAxis(axisYNormal);
        double rho = Cov/(std1*std2);
        double major_diagonal = NAN;
        double minor_diagonal = NAN;
        if (std1>=std2)
        {
            major_diagonal = 2*sqrt(2*((std1*std1+std2*std2)/2.0 + sqrt(pow((std1*std1-std2*std2)/2.0,2)+pow(rho*std1*std2,2))));
            minor_diagonal = 2*sqrt(2*((std1*std1+std2*std2)/2.0 - sqrt(pow((std1*std1-std2*std2)/2.0,2)+pow(rho*std1*std2,2))));
        }
        else
        {
            major_diagonal = 2*sqrt(2*((std1*std1+std2*std2)/2.0 - sqrt(pow((std1*std1-std2*std2)/2.0,2)+pow(rho*std1*std2,2))));
            minor_diagonal = 2*sqrt(2*((std1*std1+std2*std2)/2.0 + sqrt(pow((std1*std1-std2*std2)/2.0,2)+pow(rho*std1*std2,2))));
        }
        double orientation = 0.5*atan(2*rho*std1*std2/(std1*std1-std2*std2));

        // qDebug()<<"Std1="<<std1;
        // qDebug()<<"Std2="<<std2;
        // qDebug()<<"Rho="<<rho;
        // qDebug()<<"Major diagonal="<<major_diagonal;
        // qDebug()<<"Minor diagonal="<<minor_diagonal;
        // qDebug()<<"Orienation="<<orientation;
        vector<QPointF> ellipsepoints = calculateRotatedEllipsePoints(mean1, mean2, major_diagonal, minor_diagonal, orientation, M_PI/30.0);
        QLineSeries* seriesellipse = new QLineSeries();
        seriesellipse->setName(QString::fromStdString(vec->first));

        for (size_t i = 0; i<ellipsepoints.size(); i++)
        {
            seriesellipse->append(ellipsepoints[i].x(), ellipsepoints[i].y());
        }

        QPen penellipse = QPen(QBrush(Qt::DashLine),2,Qt::PenStyle::DashLine);
        penellipse.setColor(color);
        penellipse.setWidth(2);
        penellipse.setBrush(color);

        seriesellipse->setPen(penellipse);

        chart->addSeries(seriesellipse);

        for (size_t i = 0; i< chart->legend()->markers(seriesellipse).size(); i++)
            chart->legend()->markers(seriesellipse)[i]->setVisible(false);
        seriesellipse->attachAxis(axisX);
        seriesellipse->attachAxis(axisYNormal);


    }

    return true;
}




bool GeneralChart::PlotMCMCSamples(TimeSeries<double> *samples,const QString& variable)
{

    chart->removeAllSeries();
    for (int i=0; i<chart->axes().size(); i++)
    {
        chart->removeAxis(chart->axes()[i]);
        //delete chart->axes()[i];
    }

    for (int i=0; i<chart->axes(Qt::Horizontal).size(); i++)
    {
        chart->removeAxis(chart->axes(Qt::Horizontal)[i]);
    }

    for (int i=0; i<chart->axes(Qt::Vertical).size(); i++)
    {
        chart->removeAxis(chart->axes(Qt::Vertical)[i]);
    }
    chart->axes().clear();
    QValueAxis* axisX = new QValueAxis();
    QValueAxis* axisYNormal = new QValueAxis();
    axisX->setObjectName("axisX");
    axisYNormal->setObjectName("axisY");

    double x_min_val = samples->mint();
    double x_max_val = samples->maxt();
    double y_min_val = samples->minC();
    double y_max_val = samples->maxC();


    axisX->setRange(x_min_val, x_max_val);
    axisX->setTitleText("Sample number");
    axisX->setLabelFormat("%.0f");   // sample numbers are whole numbers

    axisYNormal->setTitleText(variable);
    PadRange(y_min_val, y_max_val);
    axisYNormal->setRange(y_min_val, y_max_val);


    QScatterSeries* series = new QScatterSeries();

    QPen marker_pen(QBrush(Qt::SolidPattern),1,Qt::PenStyle::SolidLine);

    series->setName(variable);
    series->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
    series->setMarkerSize(3.0);
    series->setPen(marker_pen);


    for (int i=0; i<samples->size(); i++)
    {
        series->append(samples->getTime(i),samples->getValue(i));
    }

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisYNormal, Qt::AlignLeft);
    chart->addSeries(series);
    series->attachAxis(axisX);
    series->attachAxis(axisYNormal);

    return true;
}


bool GeneralChart::PlotDistribution(TimeSeries<double> *samples,const QString& variable)
{
    chart->removeAllSeries();
    for (int i=0; i<chart->axes().size(); i++)
    {
        chart->removeAxis(chart->axes()[i]);
        //delete chart->axes()[i];
    }

    for (int i=0; i<chart->axes(Qt::Horizontal).size(); i++)
    {
        chart->removeAxis(chart->axes(Qt::Horizontal)[i]);
    }

    for (int i=0; i<chart->axes(Qt::Vertical).size(); i++)
    {
        chart->removeAxis(chart->axes(Qt::Vertical)[i]);
    }
    chart->axes().clear();
    QValueAxis* axisX = new QValueAxis();
    QValueAxis* axisYNormal = new QValueAxis();
    axisX->setObjectName("axisX");
    axisYNormal->setObjectName("axisY");

    double x_min_val = samples->mint();
    double x_max_val = samples->maxt();
    double y_min_val = samples->minC();
    double y_max_val = samples->maxC();


    // The x axis is the value of the variable and the y axis its posterior
    // density (both used to be labeled as if this were the samples plot)
    axisX->setRange(x_min_val, x_max_val);
    axisX->setTitleText(variable);
    axisYNormal->setTitleText("Probability density");
    axisYNormal->setRange(0, y_max_val * 1.05);

    QLineSeries *series = new QLineSeries();
    QAreaSeries *areaseries = new QAreaSeries(series);

    QPen marker_pen(QBrush(Qt::SolidPattern),1,Qt::PenStyle::SolidLine);

    series->setName(variable);
    series->setPen(marker_pen);

    for (int i=0; i<samples->size(); i++)
    {
        series-> append(samples->getTime(i),samples->getValue(i));
    }

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisYNormal, Qt::AlignLeft);

    chart->addSeries(series);
    series->attachAxis(axisX);
    series->attachAxis(axisYNormal);

    chart->addSeries(areaseries);
    areaseries->attachAxis(axisX);
    areaseries->attachAxis(axisYNormal);
    // One legend entry: the shaded posterior, named after the variable (the
    // outline and the unnamed area used to show as two boxes)
    areaseries->setName(variable);
    areaseries->setColor(QColor(0, 114, 178, 140));
    areaseries->setBorderColor(QColor(0, 114, 178));
    for (QLegendMarker* marker : chart->legend()->markers(series))
        marker->setVisible(false);

    if (result_item->Type() == result_type::distribution_with_observed)
    {
        QLineSeries *observed_series = new QLineSeries;
        QPen observed_pen(QBrush(Qt::DashLine),3,Qt::PenStyle::DashLine);
        observed_pen.setColor(Qt::black);
        observed_series->setPen(observed_pen);
        observed_series->setName("Measured in target sample");
        const double observed = static_cast<CMBTimeSeriesSet*>(result_item->Result())->ObservedValue(variable.toStdString());
        observed_series->append(observed,0);
        observed_series->append(observed,y_max_val);
        // Keep the observed value on the plot when it lies outside the
        // posterior (it used to fall off the axis)
        if (observed < x_min_val || observed > x_max_val)
        {
            double low = std::min(x_min_val, observed);
            double high = std::max(x_max_val, observed);
            PadRange(low, high);
            axisX->setRange(low, high);
        }
        chart->addSeries(observed_series);
        observed_series->attachAxis(axisX);
        observed_series->attachAxis(axisYNormal);
    }
    return true;
}

bool GeneralChart::PlotTimeSeriesSet(CMBTimeSeriesSet *timeseriesset, const QString &/*title*/, const QString &x_axis_title, const QString &y_axis_title)
{
    double x_min_val = timeseriesset->mintime();
    double x_max_val = timeseriesset->maxtime();
    double y_min_val = timeseriesset->minval();
    double y_max_val = timeseriesset->maxval();
    QString xAxisTitle = x_axis_title;
    QString yAxisTitle = y_axis_title;
    if (x_axis_title.isEmpty()) xAxisTitle = "Value";
    if (y_axis_title.isEmpty()) yAxisTitle = "CDF";
    QValueAxis* axisX = new QValueAxis();
    QValueAxis* axisY = new QValueAxis();
    axisX->setObjectName("axisX");
    axisY->setObjectName("axisY");
    axisX->setTitleText(xAxisTitle);
    axisY->setTitleText(yAxisTitle);
    axisX->setRange(x_min_val,x_max_val);
    axisY->setRange(y_min_val,y_max_val);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    for (int i=0; i<timeseriesset->size(); i++)
    {
        QLineSeries *lineseries = new QLineSeries();
        chart->addSeries(lineseries);
        lineseries->attachAxis(axisX);
        lineseries->attachAxis(axisY);

        for (int j=0; j<timeseriesset->at(i).size(); j++)
        {
            lineseries->append(timeseriesset->at(i).getTime(j),timeseriesset->at(i).getValue(j));
        }
        QPen pen = lineseries->pen();
        pen.setWidth(2);
        pen.setBrush(SeriesColor(QString::fromStdString(timeseriesset->getSeriesName(i))));
        lineseries->setPen(pen);
        lineseries->setName(QString::fromStdString(timeseriesset->getSeriesName(i)));

    }

    return true;
}

bool GeneralChart::PlotRangeSet(RangeSet *rangeset, const QString &/*title*/, const QString &x_axis_title, const QString &y_axis_title)
{
    double y_min_val = rangeset->minval();
    double y_max_val = rangeset->maxval();

    if (result_item->FixedYLimit() && result_item->YLimit(_range::high)!=0)
    {
        y_max_val = result_item->YLimit(_range::high);
    }

    if (result_item->FixedYLimit() && result_item->YLimit(_range::high)!=0)
    {
        y_min_val = result_item->YLimit(_range::low);
    }

    if (result_item->YAxisMode()==yaxis_mode::log)
    {
        // A log axis cannot reach 0, and an unset lower limit reads as 0.
        // Unless a positive lower limit was set, start the axis at the
        // decade below the smallest positive value shown (interval ends,
        // medians, observed values) instead of a fixed 1e-8, which left most
        // of the axis empty.
        if (!(result_item->FixedYLimit() && result_item->YLimit(_range::low) > 0))
        {
            const bool with_observed = (result_item->Type() == result_type::rangeset_with_observed);
            double smallest = std::numeric_limits<double>::infinity();
            for (map<string, Range>::iterator it = rangeset->begin(); it!=rangeset->end(); it++)
            {
                const double candidates[] = { it->second.Get(_range::low), it->second.Get(_range::high),
                                              it->second.Median(),
                                              with_observed ? it->second.GetValue() : 0.0 };
                for (double value : candidates)
                    if (value > 0 && std::isfinite(value))
                        smallest = min(smallest, value);
            }
            y_min_val = std::isfinite(smallest) ? std::pow(10.0, std::floor(std::log10(smallest))) : 1e-8;
        }
        if (!(y_max_val > y_min_val))
            y_max_val = y_min_val * 10.0;
    }
    QString xAxisTitle = x_axis_title;
    QString yAxisTitle = y_axis_title;
    if (x_axis_title.isEmpty()) xAxisTitle = "Constituent";
    if (y_axis_title.isEmpty()) yAxisTitle = "95% CI";
    QCategoryAxis* axisX = new QCategoryAxis();

    QLogValueAxis* axisYLog = new QLogValueAxis();
    QValueAxis* axisY = new QValueAxis();

    axisX->setObjectName("axisX");
    axisY->setObjectName("axisY");
    axisYLog->setObjectName("axisYLog");
    axisYLog->setMinorTickCount(10);

    axisX->setTitleText(xAxisTitle);
    if (result_item->YAxisMode()==yaxis_mode::log)
    {   axisYLog->setTitleText(yAxisTitle);
        axisYLog->setRange(y_min_val,y_max_val);
        axisYLog->setLabelFormat("%g");
    }
    else
    {   axisY->setTitleText(yAxisTitle);
        axisY->setRange(y_min_val,y_max_val);
        axisYLog->setLabelFormat("%f");
    }
    axisX->setRange(-0.5,rangeset->size()-0.5);


    chart->addAxis(axisX, Qt::AlignBottom);
    if (result_item->YAxisMode()==yaxis_mode::log)
        chart->addAxis(axisYLog, Qt::AlignLeft);
    else
        chart->addAxis(axisY, Qt::AlignLeft);

    double upperlimit = 0.5;
    axisX->setStartValue(-0.5);
    QBoxPlotSeries *boxWhiskSeries = new QBoxPlotSeries();

    for (map<string, Range>::iterator it = rangeset->begin(); it!=rangeset->end(); it++)
    {
        axisX->append(QString::fromStdString(it->first),upperlimit);
        upperlimit += 1;
    }

    if (result_item->Type() == result_type::rangeset_with_observed)
    {   QScatterSeries* scatterseries = new QScatterSeries();
        chart->addSeries(scatterseries);
        scatterseries->attachAxis(axisX);
        if (result_item->YAxisMode()==yaxis_mode::log)
            scatterseries->attachAxis(axisYLog);
        else
            scatterseries->attachAxis(axisY);
        int counter = 0;
        for (map<string, Range>::iterator it = rangeset->begin(); it!=rangeset->end(); it++)
        {
            scatterseries->append(counter,it->second.GetValue());
            counter++;
        }
        QPen pen = scatterseries->pen();
        pen.setWidth(2);

        pen.setBrush(QColor(Qt::black));
        scatterseries->setColor(QColor(Qt::black));
        scatterseries->setPen(pen);
        scatterseries->setName(QString::fromStdString("Observed value"));
    }

    for (map<string, Range>::iterator it = rangeset->begin(); it!=rangeset->end(); it++)
    {
        QBoxSet *boxSet = new QBoxSet();
        // qDebug()<<it->second.Get(_range::low)<<","<<it->second.Get(_range::high)<<","<<it->second.GetValue();
        boxSet->setValue(QBoxSet::LowerExtreme, max(it->second.Get(_range::low),y_min_val));
        boxSet->setValue(QBoxSet::UpperExtreme, it->second.Get(_range::high));
        boxSet->setValue(QBoxSet::LowerQuartile, max(it->second.Get(_range::low),y_min_val));
        boxSet->setValue(QBoxSet::UpperQuartile, it->second.Get(_range::high));
        boxSet->setValue(QBoxSet::Median, it->second.Median());
        boxWhiskSeries->append(boxSet);


    }
    QPen brushcolor(QColor(0,200,0,126));
    QBrush brush(QColor(0,200,0,126));
    boxWhiskSeries->setBrush(brush);
    boxWhiskSeries->setName("95% Credible Interval");

    chart->addSeries(boxWhiskSeries);
    if (result_item->YAxisMode()==yaxis_mode::log)
        boxWhiskSeries->attachAxis(axisYLog);
    else
        boxWhiskSeries->attachAxis(axisY);
    boxWhiskSeries->attachAxis(axisX);

    return true;
}

bool GeneralChart::PlotTimeSeriesSet_M(CMBTimeSeriesSet *timeseriesset, const QString &/*title*/, const QString &x_axis_title, const QString &y_axis_title)
{
    double x_min_val = timeseriesset->mintime();
    double x_max_val = timeseriesset->maxtime();
    double y_min_val = timeseriesset->minval();
    double y_max_val = timeseriesset->maxval();
    QValueAxis* axisX = new QValueAxis();
    QValueAxis* axisY = new QValueAxis();
    QString xAxisTitle = x_axis_title;
    QString yAxisTitle = y_axis_title;
    if (x_axis_title.isEmpty()) xAxisTitle = "Value";
    if (y_axis_title.isEmpty()) yAxisTitle = "CDF";
    axisX->setObjectName("axisX");
    axisY->setObjectName("axisY");
    axisX->setTitleText(xAxisTitle);
    axisY->setTitleText(yAxisTitle);
    axisX->setRange(x_min_val,x_max_val);
    axisY->setRange(y_min_val,y_max_val);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    //axisX->setStartValue(x_min_val);
    QStringList x_Labels;
    int i=0;
    {
        QScatterSeries* scatterseries = new QScatterSeries();
        chart->addSeries(scatterseries);
        scatterseries->attachAxis(axisX);
        scatterseries->attachAxis(axisY);

        for (int j=0; j<timeseriesset->at(i).size(); j++)
        {
            x_Labels.append(QString::fromStdString(timeseriesset->Label(j)));
            //axisX->append(QString::fromStdString(timeseriesset->Label(j)),j+1);
            scatterseries->append(timeseriesset->at(i).getTime(j),timeseriesset->at(i).getValue(j));
        }
        QPen pen = scatterseries->pen();
        pen.setWidth(2);

        pen.setBrush(QColor(Qt::black));
        scatterseries->setColor(QColor(Qt::black));
        scatterseries->setPen(pen);
        scatterseries->setName(QString::fromStdString(timeseriesset->getSeriesName(i)));

    }
    for (int i=1; i<timeseriesset->size(); i++)
    {
        QLineSeries *lineseries = new QLineSeries();
        chart->addSeries(lineseries);
        lineseries->attachAxis(axisX);
        lineseries->attachAxis(axisY);

        for (int j=0; j<timeseriesset->at(i).size(); j++)
        {
            lineseries->append(timeseriesset->at(i).getTime(j),timeseriesset->at(i).getValue(j));
        }
        QPen pen = lineseries->pen();
        pen.setWidth(2);
        pen.setBrush(SeriesColor(QString::fromStdString(timeseriesset->getSeriesName(i))));
        lineseries->setPen(pen);
        lineseries->setName(QString::fromStdString(timeseriesset->getSeriesName(i)));

    }

    return true;
}


bool GeneralChart::PlotTimeSeriesSet_A(CMBTimeSeriesSet *timeseriesset, const QString &/*title*/, const QString &x_axis_title, const QString &y_axis_title)
{
    double x_min_val = timeseriesset->mintime();
    double x_max_val = timeseriesset->maxtime();
    double y_min_val = result_item->YLimit(_range::low);
    double y_max_val = result_item->YLimit(_range::high);
    QCategoryAxis* axisX = new QCategoryAxis();
    QValueAxis* axisY = new QValueAxis();
    QString xAxisTitle = x_axis_title;
    QString yAxisTitle = y_axis_title;
    if (x_axis_title.isEmpty()) xAxisTitle = "Value";
    if (y_axis_title.isEmpty()) yAxisTitle = "Sample";
    axisX->setObjectName("axisX");
    axisY->setObjectName("axisY");
    axisX->setTitleText(xAxisTitle);
    axisY->setTitleText(yAxisTitle);
    axisX->setRange(x_min_val,x_max_val);
    QStringList XAxisLabels; 
    axisY->setRange(y_min_val,y_max_val);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    for (int i=0; i<timeseriesset->size(); i++)
    {
        QScatterSeries* scatterseries = new QScatterSeries();
        chart->addSeries(scatterseries);
        scatterseries->attachAxis(axisX);
        scatterseries->attachAxis(axisY);

        for (int j=0; j<timeseriesset->at(i).size(); j++)
        {
            scatterseries->append(timeseriesset->at(i).getTime(j),timeseriesset->at(i).getValue(j));
            if (i==0)
                axisX->append(QString::fromStdString(timeseriesset->Label(j)),j+0.5);
        }
        
        QPen pen = scatterseries->pen();
        pen.setWidth(2);

        pen.setBrush(SeriesColor(QString::fromStdString(timeseriesset->getSeriesName(i))));
        scatterseries->setColor(SeriesColor(QString::fromStdString(timeseriesset->getSeriesName(i))));
        scatterseries->setPen(pen);
        scatterseries->setName(QString::fromStdString(timeseriesset->getSeriesName(i)));

    }


    return true;
}

bool GeneralChart::PlotTimeSeriesSet_Stacked(CMBTimeSeriesSet *timeseriesset, const QString &title, const QString &x_axis_title, const QString &y_axis_title)
{

    QVector<QBarSet*> sets(timeseriesset->size());
    for (int i=0; i<timeseriesset->size(); i++)
    {   sets[i]  = new QBarSet(QString::fromStdString(timeseriesset->getSeriesName(i)));
        sets[i]->setColor(SeriesColor(sets[i]->label()));
        for (int j=0; j<timeseriesset->at(i).size(); j++)
            *sets[i] << timeseriesset->at(i).getValue(j);

    }

    QStackedBarSeries *series = new QStackedBarSeries;
    for (int i=0; i<timeseriesset->size(); i++)
        series->append(sets[i]);

    chart->addSeries(series);
    chart->setTitle(title);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    QStringList categories;
    for (int j=0; j<timeseriesset->at(0).size(); j++)
        categories << QString::fromStdString(timeseriesset->Label(j));

    auto axisX = new QBarCategoryAxis;
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    auto axisY = new QValueAxis;
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // The titles were worked out but never set (and the defaults swapped)
    axisX->setTitleText(x_axis_title.isEmpty() ? QString("Sample") : x_axis_title);
    axisY->setTitleText(y_axis_title.isEmpty() ? QString("Contribution") : y_axis_title);
    if (result_item->YLimit(_range::high) > result_item->YLimit(_range::low))
        axisY->setRange(result_item->YLimit(_range::low), result_item->YLimit(_range::high));
    axisX->setLabelsAngle(-90);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);


    return true;

}

void GeneralChart::on_Exporttopng()
{
    QRect rect = QDialog::frameGeometry();
    
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save"), "",
        tr("png file (*.png)"));

    if (!fileName.contains("."))
        fileName+=".png";
    chart->setAnimationOptions(QChart::NoAnimation);
    this->resize(rect.width() * 2, rect.height() * 2);

    chartView->grab().save(fileName);
    this->resize(rect.size());
}


std::vector<QPointF> GeneralChart::calculateRotatedEllipsePoints(
    double centerX, double centerY,
    double semiMajorAxis, double semiMinorAxis,
    double rotationAngle, double interval
) {
    std::vector<QPointF> points;

    // Iterate over the angle in the range [0, 2π] with the given interval
    for (double t = 0; t <= 2 * M_PI; t += interval) {
        // Parametric equations for rotated ellipse
        double x = centerX + semiMajorAxis * cos(t) * cos(rotationAngle)
                             - semiMinorAxis * sin(t) * sin(rotationAngle);
        double y = centerY + semiMajorAxis * cos(t) * sin(rotationAngle)
                             + semiMinorAxis * sin(t) * cos(rotationAngle);
        QPointF point(x,y);
        points.push_back(point);
    }


    return points;
}
