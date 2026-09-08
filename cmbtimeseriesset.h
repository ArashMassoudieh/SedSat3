#ifndef CMBTimeSeriesSet_H
#define CMBTimeSeriesSet_H

#include "TimeSeriesSet.h"
#include "interface.h"

/**
 * @class CMBTimeSeriesSet
 * @brief Collection of time series with labels and observed values
 *
 * Extends TimeSeriesSet<double> with custom labels for time points, observed values
 * for comparison, JSON serialization, and table generation. Used for organizing
 * multiple time series data such as contribution time series from different sources
 * or fitted vs observed distributions.
 */
class CMBTimeSeriesSet : public TimeSeriesSet<double>, public Interface
{
public:
    /**
     * @brief Default constructor - creates an empty time series set
     */
    CMBTimeSeriesSet();

    /**
     * @brief Constructs a time series set with given capacity
     * @param n Initial number of series
     */
    CMBTimeSeriesSet(int n);

    /**
     * @brief Constructs with specified number of series and points
     * @param n Number of series
     * @param m Number of points per series
     */
    CMBTimeSeriesSet(int n, int m);

    /**
     * @brief Copy constructor from CMBTimeSeriesSet
     * @param mp Time series set to copy from
     */
    CMBTimeSeriesSet(const CMBTimeSeriesSet& mp);

    /**
     * @brief Assignment operator from CMBTimeSeriesSet
     * @param mp Time series set to assign from
     * @return Reference to this time series set
     */
    CMBTimeSeriesSet& operator=(const CMBTimeSeriesSet& mp);

    /**
     * @brief Assignment operator from base TimeSeriesSet
     * @param mp Time series set to assign from
     * @return Reference to this time series set
     */
    CMBTimeSeriesSet& operator=(const TimeSeriesSet<double>& mp);

    /**
     * @brief Constructs from base TimeSeriesSet
     * @param mp Time series set to copy from
     */
    CMBTimeSeriesSet(const TimeSeriesSet<double>& mp);

    /**
     * @brief Serializes time series set to JSON format
     * @return JSON object containing all series, labels, and observed values
     */
    QJsonObject toJsonObject() const override;

    /**
     * @brief Deserializes time series set from JSON format
     * @param jsonobject JSON object to read from
     * @return true if successful, false otherwise
     */
    bool ReadFromJsonObject(const QJsonObject& jsonobject) override;

    /**
     * @brief Converts time series set to CSV-formatted string
     * @return String representation with all series
     */
    string ToString() const override;

    /**
     * @brief Writes time series set to file
     * @param file File pointer to write to
     * @return true if successful
     */
    bool writetofile(QFile*) override;

    /**
     * @brief Creates a QTableWidget for displaying time series set data
     *
     * Generates either a table with separate time columns for each series,
     * or a single time column with multiple value columns, depending on
     * the single_column_x option.
     *
     * @return Pointer to newly created QTableWidget (caller takes ownership)
     */
    QTableWidget* ToTable() override;

    /**
     * @brief Appends a final contribution series calculated from existing series
     *
     * Calculates the remaining contribution (1 - sum of first colnumber series)
     * and inserts it at position colnumber in the set.
     *
     * @param colnumber Number of existing contribution series
     * @param name Name for the final contribution series
     */
    void AppendLastContribution(int colnumber, const string& name);

    /**
     * @brief Sets observed value for comparison at specified index
     * @param i Series index
     * @param value Observed value
     */
    void SetObservedValue(int i, const double& value)
    {
        if (i < size())
            observed_value[i] = value;
    }

    /**
     * @brief Gets observed value at specified index
     * @param i Series index
     * @return Observed value, or 0 if index out of range
     */
    double ObservedValue(int i)
    {
        if (i < size())
            return observed_value[i];
        else
            return 0;
    }

    /**
     * @brief Gets observed value for named series
     * @param variable_name Name of the series
     * @return Observed value, or 0 if series not found
     */
    double ObservedValue(string variable_name)
    {
        for (int i = 0; i < size(); i++)
        {
            if (getSeriesName(i) == variable_name)
                return observed_value[i];
        }
        return 0;
    }

    /**
     * @brief Gets label for time point (uses custom labels if set)
     * @param i Time point index
     * @return Custom label if available, otherwise formatted time value
     */
    string Label(unsigned int i) const
    {
        if (i < labels.size())
            return labels[i];
        else if (i < maxnumpoints())
            return aquiutils::numbertostring(at(0).getTime(i));
        else
            return "";
    }

    /**
     * @brief Gets label for time point in specific series
     * @param i Time point index
     * @param j Series index
     * @return Custom label if available, otherwise formatted time value from series j
     */
    string Label(unsigned int i, unsigned int j) const
    {
        if (i < labels.size())
            return labels[i];
        else if (i < maxnumpoints())
            return aquiutils::numbertostring(at(j).getTime(i));
        else
            return "";
    }

    /**
     * @brief Sets custom label for time point
     * @param i Time point index
     * @param label Label string to set
     */
    void SetLabel(unsigned int i, const string& label)
    {
        if (i < maxnumpoints())
        {
            labels.resize(maxnumpoints());
            labels[i] = label;
        }
        else
            return;
    }

private:
    vector<double> observed_value;  ///< Observed values for comparison with modeled series
    vector<string> labels;          ///< Custom labels for time points
};

#endif // CMBTimeSeriesSet_H