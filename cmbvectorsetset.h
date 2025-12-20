#ifndef CMBVectorSetSetSET_H
#define CMBVectorSetSetSET_H

#include "interface.h"
#include "cmbvectorset.h"
#include <map>

/**
 * @class CMBVectorSetSet
 * @brief Collection of named CMBVectorSet objects for hierarchical data organization
 *
 * Manages a two-level hierarchy of vectors: sets of vector sets. Provides JSON
 * serialization and table generation for complex multi-group, multi-variable datasets.
 * Commonly used for organizing discriminant function analysis results across multiple
 * discriminant axes.
 */
class CMBVectorSetSet : public Interface, public map<string, CMBVectorSet>
{
public:
    /**
     * @brief Default constructor - creates an empty vector set set
     */
    CMBVectorSetSet();

    /**
     * @brief Copy constructor
     * @param mp Vector set set to copy from
     */
    CMBVectorSetSet(const CMBVectorSetSet& mp);

    /**
     * @brief Assignment operator
     * @param mp Vector set set to assign from
     * @return Reference to this vector set set
     */
    CMBVectorSetSet& operator=(const CMBVectorSetSet& mp);

    /**
     * @brief Serializes vector set set to JSON format
     * @return JSON object containing all vector sets with their names as keys
     */
    QJsonObject toJsonObject() const override;

    /**
     * @brief Deserializes vector set set from JSON format
     * @param jsonobject JSON object to read from
     * @return true if successful, false otherwise
     */
    bool ReadFromJsonObject(const QJsonObject& jsonobject) override;

    /**
     * @brief Converts vector set set to string representation
     * @return String containing all vector sets with their names
     */
    string ToString() const override;

    /**
     * @brief Creates a QTableWidget for displaying vector set set data
     *
     * Generates a table with column pairs for each vector in each set,
     * with hierarchical column headers.
     *
     * @return Pointer to newly created QTableWidget (caller takes ownership)
     */
    QTableWidget* ToTable() override;

    /**
     * @brief Writes vector set set to file
     * @param file File pointer to write to
     * @return true if successful
     */
    bool writetofile(QFile* file) override;

    /**
     * @brief Gets label at specified position in a column within a vector set
     * @param vectorset Name of the vector set
     * @param column Name of the vector column within the set
     * @param j Index within the column
     * @return Label string, or empty string if not found
     */
    string Label(const string& vectorset, const string& column, int j) const;

    /**
     * @brief Gets value at specified position in a column within a vector set
     * @param vectorset Name of the vector set
     * @param columnlabel Name of the vector column within the set
     * @param j Index within the column
     * @return Value at position j, or 0 if not found
     */
    double valueAt(const string& vectorset, const string& columnlabel, int j) const;

    /**
     * @brief Gets reference to a named column vector within a vector set
     * @param vectorset Name of the vector set
     * @param columnlabel Name of the vector column
     * @return Reference to the CMBVector
     */
    CMBVector& GetColumn(const string& vectorset, const string& columnlabel);

    /**
     * @brief Adds or replaces a vector set in the collection
     * @param columnlabel Name for the vector set
     * @param vectorset Vector set to add
     */
    void Append(const string& columnlabel, const CMBVectorSet& vectorset);

    /**
     * @brief Finds the maximum size among all vectors in all sets
     * @return Maximum number of elements in any vector
     */
    unsigned int MaxSize() const;

    /**
     * @brief Finds the maximum value across all vectors in all sets
     * @return Maximum value found
     */
    double max() const;

    /**
     * @brief Finds the minimum value across all vectors in all sets
     * @return Minimum value found
     */
    double min() const;

private:

};

#endif // CMBVectorSetSetSET_H