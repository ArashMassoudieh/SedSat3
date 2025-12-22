#ifndef GENERALCHARTPLOTTER_H
#define GENERALCHARTPLOTTER_H

#include <QChartView>
#include <QChart>
#include <QScatterSeries>
#include <QLineSeries>
#include <QValueAxis>
#include <QLogValueAxis>
#include <QBarCategoryAxis>
#include <QLegend>
#include <QMouseEvent>
#include <QRubberBand>
#include <vector>
#include <map>
#include <string>



/**
 * @enum AxisScale
 * @brief Defines the scale type for chart axes
 */
enum class ChartAxisScale { Normal, Logarithmic };

/**
 * @enum ChartAxis
 * @brief Identifies which axis to operate on
 */
enum class ChartAxis { X, Y };

/**
 * @class GeneralChartPlotter
 * @brief Custom chart plotter widget using Qt Charts framework
 *
 * Provides interactive plotting capabilities including:
 * - Scatter plots with multiple series
 * - Time series line plots
 * - Categorical data visualization
 * - Mouse-based zoom and pan
 * - Logarithmic and linear scales
 * - Customizable markers and colors
 *
 * Supports both numeric and categorical X-axis data.
 */
class GeneralChartPlotter : public QChartView
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget (default: nullptr)
     */
    explicit GeneralChartPlotter(QWidget* parent = nullptr);

    /**
     * @brief Destructor - cleans up chart resources
     */
    virtual ~GeneralChartPlotter();

    /**
     * @brief Removes all series and resets the chart
     */
    void Clear();

    /**
     * @brief Adds a scatter plot with numeric X-axis
     * @param name Series name for legend
     * @param x X-coordinate values
     * @param y Y-coordinate values
     * @param markerSize Size of scatter markers (default: 10)
     * @return true if successful, false if x and y sizes don't match
     */
    bool AddScatter(const std::string& name,
        const std::vector<double>& x,
        const std::vector<double>& y,
        int markerSize = 10);

    /**
     * @brief Adds a scatter plot with categorical X-axis
     * @param name Series name for legend
     * @param x Category labels for X-axis
     * @param y Y-coordinate values
     * @param markerSize Size of scatter markers (default: 10)
     * @return true if successful, false if x and y sizes don't match
     */
    bool AddScatter(const std::string& name,
        const std::vector<std::string>& x,
        const std::vector<double>& y,
        int markerSize = 10);

    /**
     * @brief Adds multiple scatter series with numeric X-axis
     * @param names Vector of series names
     * @param x Vector of X-coordinate vectors (one per series)
     * @param y Vector of Y-coordinate vectors (one per series)
     * @return true if successful, false if dimensions don't match
     */
    bool AddScatters(const std::vector<std::string>& names,
        const std::vector<std::vector<double>>& x,
        const std::vector<std::vector<double>>& y);

    /**
     * @brief Adds multiple scatter series with shared categorical X-axis
     * @param names Vector of series names
     * @param x Shared category labels for X-axis
     * @param y Vector of Y-coordinate vectors (one per series)
     * @return true if successful, false if dimensions don't match
     */
    bool AddScatters(const std::vector<std::string>& names,
        const std::vector<std::string>& x,
        const std::vector<std::vector<double>>& y);

    /**
     * @brief Adds scatter points with non-uniform distribution
     *
     * Creates a scatter plot where each category can have multiple data points
     *
     * @param data Map of category name to vector of Y values
     * @param colorIndex Index for selecting marker color (default: 0)
     * @return true if successful
     */
    bool AddNonUniformScatter(const std::map<std::string, std::vector<double>>& data,
        int colorIndex = 0,
        int markerSize = 10,
        const std::string& seriesName = "");  

    /**
     * @brief Adds a line series (time series plot)
     * @param name Series name for legend
     * @param x X-coordinate values (typically time)
     * @param y Y-coordinate values
     * @return true if successful, false if x and y sizes don't match
     */
    bool AddTimeSeries(const std::string& name,
        const std::vector<double>& x,
        const std::vector<double>& y);

    /**
     * @brief Sets the Y-axis scale type
     * @param axisScale Normal or Logarithmic scale
     * @return true if successful
     */
    bool SetYAxisScaleType(ChartAxisScale axisScale);

    /**
     * @brief Sets the X-axis scale type
     * @param axisScale Normal or Logarithmic scale
     * @return true if successful
     */
    bool SetXAxisScaleType(ChartAxisScale axisScale);

    /**
     * @brief Enables or disables zoom mode
     * @param mode true to enable zoom, false to disable
     */
    void SetZoomMode(bool mode);

    /**
     * @brief Resets zoom to show all data
     */
    void ZoomExtents();

    /**
     * @brief Sets the range for specified axis
     * @param range Vector with [min, max] values
     * @param whichAxis Which axis to set (X or Y)
     */
    void SetRange(const std::vector<double>& range, ChartAxis whichAxis);

    /**
     * @brief Shows or hides the legend
     * @param visible true to show legend, false to hide
     */
    void SetLegend(bool visible);

    /**
     * @brief Sets X-axis label
     * @param label Axis label text
     */
    void SetXAxisLabel(const std::string& label);

    /**
     * @brief Sets Y-axis label
     * @param label Axis label text
     */
    void SetYAxisLabel(const std::string& label);

    /**
     * @brief Sets chart title
     * @param title Chart title text
     */
    void SetTitle(const std::string& title);

protected:
    /**
     * @brief Handles mouse press events for zoom functionality
     * @param event Mouse event
     */
    void mousePressEvent(QMouseEvent* event) override;

    /**
     * @brief Handles mouse move events for zoom rectangle
     * @param event Mouse event
     */
    void mouseMoveEvent(QMouseEvent* event) override;

    /**
     * @brief Handles mouse release events to complete zoom
     * @param event Mouse event
     */
    void mouseReleaseEvent(QMouseEvent* event) override;

    /**
     * @brief Handles mouse wheel events for zoom
     * @param event Wheel event
     */
    void wheelEvent(QWheelEvent* event) override;

private:
    /**
     * @brief Generates a color from predefined palette
     * @param index Color index
     * @return QColor object
     */
    QColor GetColor(int index) const;

    /**
     * @brief Generates a marker shape from predefined set
     * @param index Shape index
     * @return Marker shape as QScatterSeries::MarkerShape
     */
    QScatterSeries::MarkerShape GetMarkerShape(int index) const;

    /**
     * @brief Updates axis ranges based on current data
     */
    void UpdateAxisRanges();

    
    /**
        * @brief Updates Y-range considering only positive values for log scale
        * @param values Vector of Y values to consider
        */
    void UpdateYRangeForLogScale(const std::vector<double>& values);

    /**
     * @brief Creates and configures value axes
     */
    void SetupAxes();

    QChart* chart;                          ///< Main chart object
    QValueAxis* xAxisNumeric;               ///< Numeric X-axis
    QValueAxis* yAxisNumeric;               ///< Numeric Y-axis
    QLogValueAxis* yAxisLogarithmic;        ///< Logarithmic Y-axis
    QBarCategoryAxis* xAxisCategorical;     ///< Categorical X-axis

    bool zoomMode;                          ///< Zoom mode enabled flag
    QRubberBand* rubberBand;                ///< Rubber band for zoom selection
    QPoint rubberBandOrigin;                ///< Origin point for rubber band

    std::vector<double> xRange;             ///< Current X-axis range [min, max]
    std::vector<double> yRange;             ///< Current Y-axis range [min, max]

    int seriesCount;                        ///< Number of series added
    bool usingCategoricalXAxis;             ///< Flag for categorical X-axis mode

    static const int MAX_COLORS = 20;       ///< Maximum predefined colors
    static const int MAX_SHAPES = 14;       ///< Maximum predefined shapes
};

#endif // GENERALCHARTPLOTTER_H