#ifndef CMBMATRIX_H
#define CMBMATRIX_H

#include "Matrix.h"
#include "interface.h"
#include "range.h"
#include "cmbvector.h"

/**
 * @class CMBMatrix
 * @brief Matrix class with labeled rows and columns for Chemical Mass Balance analysis
 *
 * Extends CMatrix with string labels for rows and columns, JSON serialization,
 * table widget generation, and specialized operations for CMB calculations.
 * Inherits from both CMatrix (for mathematical operations) and Interface
 * (for I/O and visualization).
 */
class CMBMatrix : public CMatrix, public Interface
{
public:
    /**
     * @brief Default constructor - creates an empty matrix
     */
    CMBMatrix();

    /**
     * @brief Constructs a square matrix with given dimension
     * @param n Number of rows and columns
     */
    CMBMatrix(int n);

    /**
     * @brief Constructs a rectangular matrix with given dimensions
     * @param n Number of columns
     * @param m Number of rows
     */
    CMBMatrix(int n, int m);

    /**
     * @brief Copy constructor
     * @param mp Matrix to copy from
     */
    CMBMatrix(const CMBMatrix& mp);

    /**
     * @brief Converts to base CMatrix type (without labels)
     * @return CMatrix representation
     */
    CMatrix toMatrix() const;

    /**
     * @brief Assignment operator
     * @param mp Matrix to assign from
     * @return Reference to this matrix
     */
    CMBMatrix& operator=(const CMBMatrix& mp);

    /**
     * @brief Serializes matrix to JSON format
     * @return JSON object containing matrix data, row labels, and column labels
     */
    QJsonObject toJsonObject() const override;

    /**
     * @brief Deserializes matrix from JSON format
     * @param jsonobject JSON object to read from
     * @return true if successful, false otherwise
     */
    bool ReadFromJsonObject(const QJsonObject& jsonobject) override;

    /**
     * @brief Converts matrix to CSV-formatted string
     * @return String representation with labels and comma-separated values
     */
    string ToString() const override;

    /**
     * @brief Writes matrix to file in CSV format
     * @param file File pointer to write to
     * @return true if successful
     */
    bool writetofile(QFile* file) override;

    /**
     * @brief Gets value at specified matrix position
     * @param i Row index
     * @param j Column index
     * @return Matrix value at position (i,j)
     */
    double valueAt(int i, int j) const;

    /**
     * @brief Gets column label at specified index
     * @param i Column index
     * @return Column label string
     */
    string ColumnLabel(int i) { return columnlabels[i]; }

    /**
     * @brief Gets row label at specified index
     * @param i Row index
     * @return Row label string
     */
    string RowLabel(int i) { return rowlabels[i]; }

    /**
     * @brief Sets column label at specified index
     * @param i Column index
     * @param label Label string to set
     */
    void SetColumnLabel(int i, const string& label) { columnlabels[i] = label; }

    /**
     * @brief Sets row label at specified index
     * @param i Row index
     * @param label Label string to set
     */
    void SetRowLabel(int i, const string& label) { rowlabels[i] = label; }

    /**
     * @brief Sets all column labels at once
     * @param label Vector of label strings
     */
    void SetColumnLabels(const vector<string>& label) { columnlabels = label; }

    /**
     * @brief Sets all row labels at once
     * @param label Vector of label strings
     */
    void SetRowLabels(const vector<string>& label) { rowlabels = label; }

    /**
     * @brief Creates a QTableWidget for displaying matrix data
     *
     * Generates a read-only table widget with labeled rows and columns.
     * Highlights cells outside limit range if highlighting is enabled.
     * Displays boolean values as "Pass"/"Fail" if boolean mode is set.
     *
     * @return Pointer to newly created QTableWidget (caller takes ownership)
     */
    QTableWidget* ToTable() override;

    /**
     * @brief Gets all row labels
     * @return Vector of row label strings
     */
    vector<string> RowLabels() const { return rowlabels; }

    /**
     * @brief Gets all column labels
     * @return Vector of column label strings
     */
    vector<string> ColumnLabels() const { return columnlabels; }

    /**
     * @brief Extracts a row by its label
     * @param rowlabel Label of the row to extract
     * @return CMBVector containing the row values with column labels
     */
    CMBVector GetRow(const string& rowlabel);

    /**
     * @brief Extracts a column by its label
     * @param columnlabel Label of the column to extract
     * @return CMBVector containing the column values with row labels
     */
    CMBVector GetColumn(const string& columnlabel);

    /**
     * @brief Gets unique row label categories
     * @return QStringList of unique row labels (no duplicates)
     */
    QStringList RowLabelCategories();

    /**
     * @brief Sets boolean display mode for table widget
     *
     * When true, matrix values are displayed as "Pass" (0) or "Fail" (1)
     * in the table widget instead of numeric values.
     *
     * @param val True to enable boolean display mode
     */
    void SetBooleanValue(bool val) { boolean_values = val; }

private:
    vector<string> columnlabels;  ///< Labels for each column
    vector<string> rowlabels;     ///< Labels for each row
    bool boolean_values = false;  ///< Display values as Pass/Fail instead of numbers
};

/**
 * @brief Matrix-vector multiplication operator
 *
 * Multiplies a CMBMatrix by a CMBVector, preserving row labels in the result.
 *
 * @param M Matrix (left operand)
 * @param V Vector (right operand)
 * @return Resulting vector with row labels from matrix M
 */
CMBVector operator*(const CMBMatrix& M, const CMBVector& V);

#endif // CMBMATRIX_H