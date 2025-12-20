#ifndef CMBVECTORSET_H
#define CMBVECTORSET_H

#include "interface.h"
#include "cmbvector.h"
#include <map>

/**
 * @class CMBVectorSet
 * @brief Collection of named CMBVector objects for multi-variable analysis
 *
 * Manages a set of labeled vectors stored as a map, providing JSON serialization,
 * table generation, and statistical operations like ANOVA F-test. Commonly used
 * for organizing multiple samples or variables in source apportionment analysis.
 */
class CMBVectorSet : public Interface, public map<string, CMBVector>
{
public:
    /**
     * @brief Default constructor - creates an empty vector set
     */
    CMBVectorSet();

    /**
     * @brief Copy constructor
     * @param mp Vector set to copy from
     */
    CMBVectorSet(const CMBVectorSet& mp);

    /**
     * @brief Assignment operator
     * @param mp Vector set to assign from
     * @return Reference to this vector set
     */
    CMBVectorSet& operator=(const CMBVectorSet& mp);

    /**
     * @brief Serializes vector set to JSON format
     * @return JSON object containing all vectors with their names as keys
     */
    QJsonObject toJsonObject() const override;

    /**
     * @brief Deserializes vector set from JSON format
     * @param jsonobject JSON object to read from
     * @return true if successful, false otherwise
     */
    bool ReadFromJsonObject(const QJsonObject& jsonobject) override;

    /**
     * @brief Converts vector set to string representation
     * @return String containing all vectors with their names
     */
    string ToString() const override;

    /**
     * @brief Creates a QTableWidget for displaying vector set data
     *
     * Generates a table with one column pair (label, value) for each vector.
     *
     * @return Pointer to newly created QTableWidget (caller takes ownership)
     */
    QTableWidget* ToTable() override;

    /**
     * @brief Writes vector set to file
     * @param file File pointer to write to
     * @return true if successful
     */
    bool writetofile(QFile* file) override;

    /**
     * @brief Gets label at specified position in a column
     * @param column Name of the vector column
     * @param j Index within the column
     * @return Label string, or empty string if column not found
     */
    string Label(string column, int j) const;

    /**
     * @brief Gets value at specified position in a column
     * @param columnlabel Name of the vector column
     * @param j Index within the column
     * @return Value at position j, or 0 if column not found
     */
    double valueAt(const string& columnlabel, int j) const;

    /**
     * @brief Gets reference to a named column vector
     * @param columnlabel Name of the vector column
     * @return Reference to the CMBVector
     */
    CMBVector& GetColumn(const string columnlabel);

    /**
     * @brief Adds or replaces a vector in the set
     * @param columnlabel Name for the vector
     * @param vectorset Vector to add
     */
    void Append(const string& columnlabel, const CMBVector& vectorset);

    /**
     * @brief Finds the maximum size among all vectors
     * @return Maximum number of elements in any vector
     */
    unsigned int MaxSize() const;

    /**
     * @brief Finds the maximum value across all vectors
     * @return Maximum value found
     */
    double max() const;

    /**
     * @brief Finds the minimum value across all vectors
     * @return Minimum value found
     */
    double min() const;

    /**
     * @brief Computes F-test p-value for ANOVA
     *
     * Performs one-way ANOVA to test if means differ significantly
     * across the vectors in the set.
     *
     * @return F-test p-value
     */
    double FTest_p_value() const;

    /**
     * @brief Computes overall mean across all vectors
     *
     * Calculates weighted mean across all vectors, accounting for
     * different vector sizes.
     *
     * @return Overall mean value
     */
    double OverallMean() const;

    /**
     * @brief Counts total number of observations across all vectors
     * @return Total number of elements in all vectors combined
     */
    int TotalNumberofObservations() const;

private:

};

#endif // CMBVECTORSET_H