#ifndef CMBTimeSeries_H
#define CMBTimeSeries_H

#include "interface.h"

/**
 * @class CMBTimeSeries
 * @brief Time series data container with Interface support
 *
 * Extends TimeSeries<double> with JSON serialization, file I/O, and table
 * widget generation. Used for storing time-indexed data such as concentration
 * profiles or temporal distributions.
 */
class CMBTimeSeries : public TimeSeries<double>, public Interface
{
public:
    /**
     * @brief Default constructor - creates an empty time series
     */
    CMBTimeSeries();

    /**
     * @brief Constructs a time series with given capacity
     * @param n Initial capacity
     */
    CMBTimeSeries(int n);

    /**
     * @brief Copy constructor
     * @param mp Time series to copy from
     */
    CMBTimeSeries(const CMBTimeSeries& mp);

    /**
     * @brief Assignment operator
     * @param mp Time series to assign from
     * @return Reference to this time series
     */
    CMBTimeSeries& operator=(const CMBTimeSeries& mp);

    /**
     * @brief Serializes time series to JSON format
     * @return JSON object containing time and value arrays
     */
    QJsonObject toJsonObject() const override;

    /**
     * @brief Deserializes time series from JSON format
     * @param jsonobject JSON object to read from
     * @return true if successful, false otherwise
     */
    bool ReadFromJsonObject(const QJsonObject& jsonobject) override;

    /**
     * @brief Converts time series to CSV-formatted string
     * @return String representation with time,value pairs
     */
    string ToString() const override;

    /**
     * @brief Writes time series to file
     * @param file File pointer to write to
     * @return true if successful
     */
    bool writetofile(QFile*) override;

    /**
     * @brief Creates a QTableWidget for displaying time series data
     *
     * Generates a single-column table with time values as row labels
     * and values in the column. Highlights values outside limit range
     * if highlighting is enabled.
     *
     * @return Pointer to newly created QTableWidget (caller takes ownership)
     */
    QTableWidget* ToTable() override;

private:

};

#endif // CMBTimeSeries_H