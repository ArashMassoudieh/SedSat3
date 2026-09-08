#include "GeneralChartPlotter.h"
#include "Utilities.h"
#include <QRandomGenerator>
#include <algorithm>

GeneralChartPlotter::GeneralChartPlotter(QWidget* parent)
    : QChartView(parent)
    , chart(new QChart())
    , xAxisNumeric(nullptr)
    , yAxisNumeric(nullptr)
    , yAxisLogarithmic(nullptr)
    , xAxisCategorical(nullptr)
    , zoomMode(true)
    , rubberBand(new QRubberBand(QRubberBand::Rectangle, this))
    , seriesCount(0)
    , usingCategoricalXAxis(false)
{
    // Initialize ranges with extreme values that will be replaced by real data
    xRange = { 1e12, -1e12 };
    yRange = { 1e12, -1e12 };

    // Configure chart
    setChart(chart);
    chart->setAnimationOptions(QChart::NoAnimation);
    chart->setMargins(QMargins(2, 2, 2, 2));
    // Setup default axes
    SetupAxes();

    // Enable interactions
    setRenderHint(QPainter::Antialiasing);
    setRubberBand(QChartView::RectangleRubberBand);
}

GeneralChartPlotter::~GeneralChartPlotter()
{
    delete rubberBand;
    // chart is deleted by QChartView
}

void GeneralChartPlotter::SetupAxes()
{
    // Create numeric axes
    xAxisNumeric = new QValueAxis();
    yAxisNumeric = new QValueAxis();

    xAxisNumeric->setTitleText("X Axis");
    yAxisNumeric->setTitleText("Y Axis");

    chart->addAxis(xAxisNumeric, Qt::AlignBottom);
    chart->addAxis(yAxisNumeric, Qt::AlignLeft);
}

void GeneralChartPlotter::Clear()
{
    chart->removeAllSeries();

    // Reset axes
    if (xAxisCategorical) {
        chart->removeAxis(xAxisCategorical);
        delete xAxisCategorical;
        xAxisCategorical = nullptr;
    }

    if (xAxisNumeric) {
        chart->removeAxis(xAxisNumeric);
        delete xAxisNumeric;
        xAxisNumeric = nullptr;
    }

    if (yAxisNumeric) {
        chart->removeAxis(yAxisNumeric);
        delete yAxisNumeric;
        yAxisNumeric = nullptr;
    }

    if (yAxisLogarithmic) {
        chart->removeAxis(yAxisLogarithmic);
        delete yAxisLogarithmic;
        yAxisLogarithmic = nullptr;
    }

    // Recreate axes
    SetupAxes();

    // Reset ranges
    xRange = { 1e12, -1e12 };
    yRange = { 1e12, -1e12 };
    seriesCount = 0;
    usingCategoricalXAxis = false;
}

QColor GeneralChartPlotter::GetColor(int index) const
{
    static const QColor colors[] = {
        QColor(31, 119, 180),   // Blue
        QColor(255, 127, 14),   // Orange
        QColor(44, 160, 44),    // Green
        QColor(214, 39, 40),    // Red
        QColor(148, 103, 189),  // Purple
        QColor(140, 86, 75),    // Brown
        QColor(227, 119, 194),  // Pink
        QColor(127, 127, 127),  // Gray
        QColor(188, 189, 34),   // Olive
        QColor(23, 190, 207),   // Cyan
        QColor(174, 199, 232),  // Light blue
        QColor(255, 187, 120),  // Light orange
        QColor(152, 223, 138),  // Light green
        QColor(255, 152, 150),  // Light red
        QColor(197, 176, 213),  // Light purple
        QColor(196, 156, 148),  // Light brown
        QColor(247, 182, 210),  // Light pink
        QColor(199, 199, 199),  // Light gray
        QColor(219, 219, 141),  // Light olive
        QColor(158, 218, 229)   // Light cyan
    };

    return colors[index % MAX_COLORS];
}

QScatterSeries::MarkerShape GeneralChartPlotter::GetMarkerShape(int index) const
{
    static const QScatterSeries::MarkerShape shapes[] = {
        QScatterSeries::MarkerShapeCircle,
        QScatterSeries::MarkerShapeRectangle,
        QScatterSeries::MarkerShapeCircle,
        QScatterSeries::MarkerShapeRectangle,
        QScatterSeries::MarkerShapeCircle,
        QScatterSeries::MarkerShapeRectangle,
        QScatterSeries::MarkerShapeCircle,
        QScatterSeries::MarkerShapeRectangle,
        QScatterSeries::MarkerShapeCircle,
        QScatterSeries::MarkerShapeRectangle,
        QScatterSeries::MarkerShapeCircle,
        QScatterSeries::MarkerShapeRectangle,
        QScatterSeries::MarkerShapeCircle,
        QScatterSeries::MarkerShapeRectangle
    };

    return shapes[index % MAX_SHAPES];
}

bool GeneralChartPlotter::AddScatter(const std::string& name,
    const std::vector<double>& x,
    const std::vector<double>& y,
    int markerSize)
{
    if (x.size() != y.size()) {
        return false;
    }

    if (x.empty()) {
        return false;
    }

    // Update X range (always uses all values)
    double xMin = *std::min_element(x.begin(), x.end());
    double xMax = *std::max_element(x.begin(), x.end());

    if (seriesCount == 0) {
        xRange = { xMin, xMax };
    }
    else {
        xRange[0] = std::min(xRange[0], xMin);
        xRange[1] = std::max(xRange[1], xMax);
    }

    // Update Y range based on axis type
    UpdateYRangeForLogScale(y);

    // Create scatter series
    QScatterSeries* series = new QScatterSeries();
    series->setName(QString::fromStdString(name));
    series->setMarkerSize(markerSize);
    series->setMarkerShape(GetMarkerShape(seriesCount));
    series->setColor(GetColor(seriesCount));

    // Add data points - skip negative/zero values if using log scale
    for (size_t i = 0; i < x.size(); ++i) {
        if (yAxisLogarithmic && y[i] <= 0) {
            continue; // Skip invalid values for log scale
        }
        series->append(x[i], y[i]);
    }

    // Add series to chart FIRST
    chart->addSeries(series);

    // Then attach to existing axes
    if (xAxisNumeric) {
        series->attachAxis(xAxisNumeric);
    }
    if (yAxisNumeric) {
        series->attachAxis(yAxisNumeric);
    }
    else if (yAxisLogarithmic) {
        series->attachAxis(yAxisLogarithmic);
    }

    // Update axis ranges
    UpdateAxisRanges();

    ++seriesCount;
    return true;
}

bool GeneralChartPlotter::AddScatter(const std::string& name,
    const std::vector<std::string>& x,
    const std::vector<double>& y,
    int markerSize)
{
    if (x.size() != y.size()) {
        return false;
    }

    if (x.empty()) {
        return false;
    }

    // Switch to categorical X-axis if not already
    if (!usingCategoricalXAxis) {
        chart->removeAxis(xAxisNumeric);
        delete xAxisNumeric;
        xAxisNumeric = nullptr;

        xAxisCategorical = new QBarCategoryAxis();
        chart->addAxis(xAxisCategorical, Qt::AlignBottom);
        usingCategoricalXAxis = true;
    }

    // Update Y range based on axis type
    UpdateYRangeForLogScale(y);

    // ONLY add categories on the first series
    if (seriesCount == 0) {
        QStringList categories;
        for (const auto& cat : x) {
            categories << QString::fromStdString(cat);
        }
        xAxisCategorical->append(categories);
    }

    // Create scatter series
    QScatterSeries* series = new QScatterSeries();
    series->setName(QString::fromStdString(name));
    series->setMarkerSize(markerSize);
    series->setMarkerShape(GetMarkerShape(seriesCount));

    // Set color and make sure it's visible
    QColor color = GetColor(seriesCount);
    series->setColor(color);
    series->setBorderColor(color);

    // Add data points - skip negative/zero values if using log scale
    for (size_t i = 0; i < y.size(); ++i) {
        if (yAxisLogarithmic && y[i] <= 0) {
            continue; // Skip invalid values for log scale
        }
        series->append(static_cast<double>(i), y[i]);
    }

    // Add series to chart FIRST
    chart->addSeries(series);

    // Then attach axes
    series->attachAxis(xAxisCategorical);
    if (yAxisNumeric) {
        series->attachAxis(yAxisNumeric);
    }
    else if (yAxisLogarithmic) {
        series->attachAxis(yAxisLogarithmic);
    }

    // Update Y-axis range
    UpdateAxisRanges();

    ++seriesCount;
    return true;
}

bool GeneralChartPlotter::AddScatters(const std::vector<std::string>& names,
    const std::vector<std::vector<double>>& x,
    const std::vector<std::vector<double>>& y)
{
    if (names.size() != x.size() || names.size() != y.size()) {
        return false;
    }

    for (size_t i = 0; i < names.size(); ++i) {
        if (!AddScatter(names[i], x[i], y[i],3)) {
            return false;
        }
    }

    return true;
}

bool GeneralChartPlotter::AddScatters(const std::vector<std::string>& names,
    const std::vector<std::string>& x,
    const std::vector<std::vector<double>>& y)
{
    if (names.size() != y.size()) {
        return false;
    }

    for (size_t i = 0; i < names.size(); ++i) {
        if (y[i].size() != x.size()) {
            return false;
        }
        if (!AddScatter(names[i], x, y[i],3)) {
            return false;
        }
    }

    return true;
}

bool GeneralChartPlotter::AddNonUniformScatter(const std::map<std::string, std::vector<double>>& data,
    int colorIndex,
    int markerSize,
    const std::string& seriesName)  // Add parameter
{
    if (data.empty()) {
        return false;
    }

    // Switch to categorical X-axis if not already
    if (!usingCategoricalXAxis) {
        chart->removeAxis(xAxisNumeric);
        delete xAxisNumeric;
        xAxisNumeric = nullptr;

        xAxisCategorical = new QBarCategoryAxis();
        chart->addAxis(xAxisCategorical, Qt::AlignBottom);
        usingCategoricalXAxis = true;
    }

    QScatterSeries* series = new QScatterSeries();

    // Set series name for legend
    QString qSeriesName = QString::fromStdString(seriesName).trimmed();
    if (qSeriesName.isEmpty()) {
        qSeriesName = QString("Series %1").arg(seriesCount + 1);
    }
    series->setName(qSeriesName);  // ADD THIS

    series->setMarkerSize(markerSize);
    series->setMarkerShape(GetMarkerShape(colorIndex));
    series->setColor(GetColor(colorIndex));

    QStringList categories;
    int xPosition = 0;

    for (const auto& [category, values] : data) {
        categories << QString::fromStdString(category);

        // Update range based on axis type
        UpdateYRangeForLogScale(values);

        for (double value : values) {
            if (yAxisLogarithmic && value <= 0) {
                continue; // Skip invalid values for log scale
            }
            series->append(static_cast<double>(xPosition), value);
        }

        ++xPosition;
    }

    xAxisCategorical->append(categories);

    // Add series to chart FIRST
    chart->addSeries(series);

    // Then attach axes
    series->attachAxis(xAxisCategorical);
    if (yAxisNumeric) {
        series->attachAxis(yAxisNumeric);
    }
    else if (yAxisLogarithmic) {
        series->attachAxis(yAxisLogarithmic);
    }

    // Update axis ranges
    UpdateAxisRanges();

    ++seriesCount;
    return true;
}

bool GeneralChartPlotter::AddTimeSeries(const std::string& name,
    const std::vector<double>& x,
    const std::vector<double>& y)
{
    if (x.size() != y.size()) {
        return false;
    }

    if (x.empty()) {
        return false;
    }

    // Update X range
    double xMin = *std::min_element(x.begin(), x.end());
    double xMax = *std::max_element(x.begin(), x.end());

    if (seriesCount == 0) {
        xRange = { xMin, xMax * 1.1 };
    }
    else {
        xRange[0] = std::min(xRange[0], xMin);
        xRange[1] = std::max(xRange[1], xMax * 1.1);
    }

    // Update Y range based on axis type
    UpdateYRangeForLogScale(y);

    // Create line series
    QLineSeries* series = new QLineSeries();
    series->setName(QString::fromStdString(name));

    QPen pen(GetColor(seriesCount));
    pen.setWidth(3);
    series->setPen(pen);

    // Add data points - skip negative/zero values if using log scale
    for (size_t i = 0; i < x.size(); ++i) {
        if (yAxisLogarithmic && y[i] <= 0) {
            continue; // Skip invalid values for log scale
        }
        series->append(x[i], y[i]);
    }

    // Add series to chart FIRST
    chart->addSeries(series);

    // Then attach to existing axes
    if (xAxisNumeric) {
        series->attachAxis(xAxisNumeric);
    }
    if (yAxisNumeric) {
        series->attachAxis(yAxisNumeric);
    }
    else if (yAxisLogarithmic) {
        series->attachAxis(yAxisLogarithmic);
    }

    // Update axis ranges
    UpdateAxisRanges();

    ++seriesCount;
    return true;
}

bool GeneralChartPlotter::SetYAxisScaleType(ChartAxisScale axisScale)
{
    if (axisScale == ChartAxisScale::Logarithmic) {
        // Remove linear axis if it exists
        if (yAxisNumeric) {
            chart->removeAxis(yAxisNumeric);
            delete yAxisNumeric;
            yAxisNumeric = nullptr;
        }

        // Create log axis if it doesn't exist
        if (!yAxisLogarithmic) {
            yAxisLogarithmic = new QLogValueAxis();
            yAxisLogarithmic->setBase(10);
            yAxisLogarithmic->setTitleText("Y Axis (log)");

            // Reset yRange to recalculate with positive values only
            yRange = { 1e12, -1e12 };

            // Recalculate range and filter data from existing series
            for (auto* abstractSeries : chart->series()) {
                if (QScatterSeries* scatterSeries = qobject_cast<QScatterSeries*>(abstractSeries)) {
                    QList<QPointF> points = scatterSeries->points();
                    std::vector<double> yValues;
                    for (const QPointF& point : points) {
                        if (point.y() > 0) {
                            yValues.push_back(point.y());
                        }
                    }
                    UpdateYRangeForLogScale(yValues);

                    // Remove negative/zero points
                    scatterSeries->clear();
                    for (const QPointF& point : points) {
                        if (point.y() > 0) {
                            scatterSeries->append(point);
                        }
                    }
                }
                else if (QLineSeries* lineSeries = qobject_cast<QLineSeries*>(abstractSeries)) {
                    QList<QPointF> points = lineSeries->points();
                    std::vector<double> yValues;
                    for (const QPointF& point : points) {
                        if (point.y() > 0) {
                            yValues.push_back(point.y());
                        }
                    }
                    UpdateYRangeForLogScale(yValues);

                    // Remove negative/zero points
                    lineSeries->clear();
                    for (const QPointF& point : points) {
                        if (point.y() > 0) {
                            lineSeries->append(point);
                        }
                    }
                }
            }

            chart->addAxis(yAxisLogarithmic, Qt::AlignLeft);

            // Reattach all series
            for (auto* series : chart->series()) {
                series->attachAxis(yAxisLogarithmic);
            }

            // Set the range
            UpdateAxisRanges();
        }
    }
    else {
        // Remove log axis if it exists
        if (yAxisLogarithmic) {
            chart->removeAxis(yAxisLogarithmic);
            delete yAxisLogarithmic;
            yAxisLogarithmic = nullptr;
        }

        // Create linear axis if it doesn't exist
        if (!yAxisNumeric) {
            yAxisNumeric = new QValueAxis();
            yAxisNumeric->setTitleText("Y Axis");
            chart->addAxis(yAxisNumeric, Qt::AlignLeft);

            // Reattach all series
            for (auto* series : chart->series()) {
                series->attachAxis(yAxisNumeric);
            }
        }

        UpdateAxisRanges();
    }

    return true;
}

bool GeneralChartPlotter::SetXAxisScaleType(ChartAxisScale axisScale)
{
    if (usingCategoricalXAxis) {
        return false; // Can't change scale on categorical axis
    }

    if (axisScale == ChartAxisScale::Logarithmic) {
        // Replace with logarithmic axis
        chart->removeAxis(xAxisNumeric);
        delete xAxisNumeric;

        QLogValueAxis* logAxis = new QLogValueAxis();
        logAxis->setBase(10);
        logAxis->setTitleText("X Axis (log)");
        chart->addAxis(logAxis, Qt::AlignBottom);

        // Reattach all series
        for (auto* series : chart->series()) {
            series->attachAxis(logAxis);
        }

        xAxisNumeric = nullptr; // Now using log axis
    }
    else {
        // Use linear axis (default)
        if (!xAxisNumeric) {
            xAxisNumeric = new QValueAxis();
            xAxisNumeric->setTitleText("X Axis");
            chart->addAxis(xAxisNumeric, Qt::AlignBottom);

            // Reattach all series
            for (auto* series : chart->series()) {
                series->attachAxis(xAxisNumeric);
            }
        }
    }

    return true;
}

void GeneralChartPlotter::SetZoomMode(bool mode)
{
    zoomMode = mode;
}

void GeneralChartPlotter::ZoomExtents()
{
    if (xAxisNumeric) {
        xAxisNumeric->setRange(xRange[0], xRange[1]);
    }
    if (yAxisNumeric) {
        yAxisNumeric->setRange(yRange[0], yRange[1]);
    }
    else if (yAxisLogarithmic) {
        double minRange = yRange[0];
        double maxRange = yRange[1];

        // Ensure valid range for log scale
        if (minRange <= 0 || minRange >= 1e12) {
            minRange = 0.001;
        }
        if (maxRange <= minRange || maxRange <= -1e12) {
            maxRange = minRange * 1000;
        }

        yAxisLogarithmic->setRange(minRange, maxRange);
    }
}

void GeneralChartPlotter::SetRange(const std::vector<double>& range, ChartAxis whichAxis)
{
    if (range.size() != 2) {
        return;
    }

    if (whichAxis == ChartAxis::X && xAxisNumeric) {
        xAxisNumeric->setRange(range[0], range[1]);
    }
    else if (whichAxis == ChartAxis::Y) {
        if (yAxisNumeric) {
            yAxisNumeric->setRange(range[0], range[1]);
        }
        else if (yAxisLogarithmic) {
            double minRange = range[0];
            double maxRange = range[1];

            // Ensure valid range for log scale
            if (minRange <= 0) {
                minRange = 0.001;
            }
            if (maxRange <= minRange) {
                maxRange = minRange * 1000;
            }

            yAxisLogarithmic->setRange(minRange, maxRange);
        }
    }
}

void GeneralChartPlotter::SetLegend(bool visible)
{
    chart->legend()->setVisible(visible);

    if (visible) {
        chart->legend()->setAlignment(Qt::AlignTop);
        chart->legend()->setBackgroundVisible(true);

        QFont legendFont;
        legendFont.setPointSize(10);
        chart->legend()->setFont(legendFont);
    }
}

void GeneralChartPlotter::SetXAxisLabel(const std::string& label)
{
    if (xAxisNumeric) {
        xAxisNumeric->setTitleText(QString::fromStdString(label));
    }
    else if (xAxisCategorical) {
        xAxisCategorical->setTitleText(QString::fromStdString(label));
    }
}

void GeneralChartPlotter::SetYAxisLabel(const std::string& label)
{
    if (yAxisNumeric) {
        yAxisNumeric->setTitleText(QString::fromStdString(label));
    }
    else if (yAxisLogarithmic) {
        yAxisLogarithmic->setTitleText(QString::fromStdString(label));
    }
}

void GeneralChartPlotter::SetTitle(const std::string& title)
{
    chart->setTitle(QString::fromStdString(title));
}

void GeneralChartPlotter::UpdateAxisRanges()
{
    if (xAxisNumeric && xRange[0] < xRange[1]) {
        xAxisNumeric->setRange(xRange[0], xRange[1]);
    }

    if (yAxisNumeric && yRange[0] < yRange[1]) {
        yAxisNumeric->setRange(yRange[0], yRange[1]);
    }
    else if (yAxisLogarithmic) {
        double minRange = yRange[0];
        double maxRange = yRange[1];

        // Ensure valid range for log scale
        if (minRange <= 0 || minRange >= 1e12) {
            minRange = 0.001;
        }
        if (maxRange <= minRange || maxRange <= -1e12) {
            maxRange = minRange * 1000;
        }

        yAxisLogarithmic->setRange(minRange, maxRange);
    }
}

void GeneralChartPlotter::mousePressEvent(QMouseEvent* event)
{
    if (zoomMode && event->button() == Qt::LeftButton) {
        rubberBandOrigin = event->pos();
        rubberBand->setGeometry(QRect(rubberBandOrigin, QSize()));
        rubberBand->show();
    }
    QChartView::mousePressEvent(event);
}

void GeneralChartPlotter::mouseMoveEvent(QMouseEvent* event)
{
    if (rubberBand->isVisible()) {
        rubberBand->setGeometry(QRect(rubberBandOrigin, event->pos()).normalized());
    }
    QChartView::mouseMoveEvent(event);
}

void GeneralChartPlotter::mouseReleaseEvent(QMouseEvent* event)
{
    if (rubberBand->isVisible()) {
        rubberBand->hide();

        QRect rect = rubberBand->geometry();
        if (rect.width() > 5 && rect.height() > 5) {
            QRectF chartRect = chart->mapToScene(rect).boundingRect();
            chart->zoomIn(chartRect);
        }
    }
    QChartView::mouseReleaseEvent(event);
}

void GeneralChartPlotter::wheelEvent(QWheelEvent* event)
{
    qreal factor = event->angleDelta().y() > 0 ? 0.9 : 1.1;
    chart->zoom(factor);
    event->accept();
}

void GeneralChartPlotter::UpdateYRangeForLogScale(const std::vector<double>& values)
{
    if (yAxisLogarithmic) {
        // Only consider positive values for log scale
        std::vector<double> positiveValues;
        for (double val : values) {
            if (val > 0) {
                positiveValues.push_back(val);
            }
        }

        if (!positiveValues.empty()) {
            double yMin = *std::min_element(positiveValues.begin(), positiveValues.end());
            double yMax = *std::max_element(positiveValues.begin(), positiveValues.end());

            // Initialize or update range
            if (yRange[0] >= 1e12) {
                yRange[0] = yMin;
            }
            else if (yRange[0] > 0) {
                yRange[0] = std::min(yRange[0], yMin);
            }
            else {
                yRange[0] = yMin;
            }

            if (yRange[1] <= -1e12) {
                yRange[1] = yMax;
            }
            else {
                yRange[1] = std::max(yRange[1], yMax);
            }
        }
    }
    else {
        // Linear scale - use all values
        if (!values.empty()) {
            double yMin = *std::min_element(values.begin(), values.end());
            double yMax = *std::max_element(values.begin(), values.end());

            if (yRange[0] >= 1e12) {
                yRange[0] = yMin;
            }
            else {
                yRange[0] = std::min(yRange[0], yMin);
            }

            if (yRange[1] <= -1e12) {
                yRange[1] = yMax;
            }
            else {
                yRange[1] = std::max(yRange[1], yMax);
            }
        }
    }
}