#ifndef CMBVector_H
#define CMBVector_H

#include "Vector.h"
#include "interface.h"
#include "range.h"

/**
 * @class CMBVector
 * @brief Vector class with string labels for Chemical Mass Balance analysis
 *
 * Extends CVector with element labels, JSON serialization, table widget generation,
 * and specialized operations for CMB calculations. Supports both numeric and boolean
 * display modes, sorting operations, and range-based extraction.
 */
class CMBVector : public CVector, public Interface
{
public:
    /**
     * @brief Default constructor - creates an empty vector
     */
    CMBVector();

    /**
     * @brief Constructs a vector with given size
     * @param n Number of elements
     */
    CMBVector(int n);

    /**
     * @brief Copy constructor from CMBVector
     * @param mp Vector to copy from
     */
    CMBVector(const CMBVector& mp);

    /**
     * @brief Constructs from Armadillo vector
     * @param mp Armadillo vector to copy from
     */
    CMBVector(const CVector_arma& mp);

    /**
     * @brief Assignment operator from CMBVector
     * @param mp Vector to assign from
     * @return Reference to this vector
     */
    CMBVector& operator=(const CMBVector& mp);

    /**
     * @brief Constructs from base CVector
     * @param mp Base vector to copy from
     */
    CMBVector(const CVector& mp);

    /**
     * @brief Assignment operator from base CVector
     * @param mp Vector to assign from
     * @return Reference to this vector
     */
    CMBVector& operator=(const CVector& mp);

    /**
     * @brief Assignment operator from Armadillo vector
     * @param mp Armadillo vector to assign from
     * @return Reference to this vector
     */
    CMBVector& operator=(const CVector_arma& mp);

    /**
     * @brief Serializes vector to JSON format
     * @return JSON object containing vector data and labels
     */
    QJsonObject toJsonObject() const override;

    /**
     * @brief Deserializes vector from JSON format
     * @param jsonobject JSON object to read from
     * @return true if successful, false otherwise
     */
    bool ReadFromJsonObject(const QJsonObject& jsonobject) override;

    /**
     * @brief Converts vector to CSV-formatted string
     * @return String representation with labels and values
     */
    string ToString() const override;

    /**
     * @brief Creates a QTableWidget for displaying vector data
     *
     * Generates a read-only table widget with labeled rows.
     * Highlights values outside limit range if highlighting is enabled.
     * Displays as "Pass"/"Fail" if boolean mode is set.
     *
     * @return Pointer to newly created QTableWidget (caller takes ownership)
     */
    QTableWidget* ToTable() override;

    /**
     * @brief Writes vector to file in CSV format
     * @param file File pointer to write to
     * @return true if successful
     */
    bool writetofile(QFile* file) override;

    /**
     * @brief Gets value at specified index
     * @param i Element index
     * @return Value at position i
     */
    double valueAt(int i) const;

    /**
     * @brief Gets label at specified index
     * @param i Element index
     * @return Label string at position i
     */
    string Label(int i) const { return labels[i]; }

    /**
     * @brief Gets value by element label
     * @param label Element label to find
     * @return Value corresponding to label, or -999 if not found
     */
    double valueAt(const string& label) const;

    /**
     * @brief Finds index of element with given label
     * @param label Element label to find
     * @return Index of element, or -1 if not found
     */
    int LookupLabel(const string& label) const;

    /**
     * @brief Gets all element labels
     * @return Vector of label strings
     */
    vector<string> Labels() const { return labels; }

    /**
     * @brief Sets label at specified index
     * @param i Element index
     * @param label Label string to set
     */
    void SetLabel(int i, const string& label) { labels[i] = label; }

    /**
     * @brief Sets all element labels at once
     * @param label Vector of label strings
     */
    void SetLabels(const vector<string>& label) { labels = label; }

    /**
     * @brief Sets boolean display mode for table widget
     *
     * When true, values are displayed as "Pass" (0) or "Fail" (1)
     * instead of numeric values.
     *
     * @param val True to enable boolean display mode
     */
    void SetBooleanValue(bool val) { boolean_values = val; }

    /**
     * @brief Converts to base CVector type (without labels)
     * @return CVector representation
     */
    CVector toVector() const;

    /**
     * @brief Sorts vector by values in descending order
     * @param sortvector Optional vector to use for sorting order (default: sort by self)
     * @return New sorted CMBVector with labels preserved
     */
    CMBVector Sort(const CMBVector& sortvector = CMBVector()) const;

    /**
     * @brief Sorts vector by absolute values in descending order
     * @param sortvector Optional vector to use for sorting order (default: sort by self)
     * @return New sorted CMBVector with labels preserved
     */
    CMBVector AbsSort(const CMBVector& sortvector = CMBVector()) const;

    /**
     * @brief Finds label of element with maximum value
     * @return Label of element with maximum value
     */
    string MaxElement() const;

    /**
     * @brief Finds label of element with maximum absolute value
     * @return Label of element with maximum absolute value
     */
    string MaxAbsElement()const;

    /**
     * @brief Creates new vector with specified element removed
     * @param element Label of element to remove
     * @return New CMBVector without the specified element
     */
    CMBVector Eliminate(const string& element) const;

    /**
     * @brief Appends a labeled element to the vector
     * @param label Element label
     * @param val Element value
     */
    void append(const string& label, const double& val);

    /**
     * @brief Extracts a range of elements by index
     * @param start Starting index (inclusive)
     * @param end Ending index (inclusive)
     * @return New CMBVector containing elements from start to end
     */
    CMBVector Extract(int start, int end) const;

    /**
     * @brief Gets number of elements in vector
     * @return Number of elements
     */
    int size() const { return num; }

    /**
     * @brief Extracts elements with values within specified range
     * @param lowval Lower bound (exclusive)
     * @param highval Upper bound (exclusive)
     * @return New CMBVector containing elements within range
     */
    CMBVector ExtractWithinRange(const double& lowval, const double& highval) const;

    /**
     * @brief Extracts elements up to first minimum value
     *
     * Returns elements from start until the first element whose value
     * is less than the previous element's value.
     *
     * @return New CMBVector containing elements up to minimum
     */
    CMBVector ExtractUpToMinimum() const;

private:
    vector<string> labels;        ///< Labels for each element
    bool boolean_values = false;  ///< Display values as Pass/Fail instead of numbers
};

/**
 * @brief Vector addition operator
 * @param V1 First vector
 * @param V2 Second vector
 * @return Sum of vectors with labels from V1
 */
CMBVector operator+(const CMBVector&, const CMBVector&);

/**
 * @brief Scalar-vector addition operator (scalar + vector)
 * @param d Scalar value
 * @param V1 Vector
 * @return Vector with scalar added to each element
 */
CMBVector operator+(double, const CMBVector&);

/**
 * @brief Vector-scalar addition operator (vector + scalar)
 * @param V1 Vector
 * @param d Scalar value
 * @return Vector with scalar added to each element
 */
CMBVector operator+(const CMBVector&, double);

/**
 * @brief Vector subtraction operator
 * @param V1 First vector
 * @param V2 Second vector
 * @return Difference of vectors with labels from V1
 */
CMBVector operator-(const CMBVector&, const CMBVector&);

/**
 * @brief Scalar-vector subtraction operator (scalar - vector)
 * @param d Scalar value
 * @param V1 Vector
 * @return Vector with each element subtracted from scalar
 */
CMBVector operator-(double, const CMBVector&);

/**
 * @brief Vector-scalar subtraction operator (vector - scalar)
 * @param V1 Vector
 * @param d Scalar value
 * @return Vector with scalar subtracted from each element
 */
CMBVector operator-(const CMBVector&, double);

/**
 * @brief Element-wise vector multiplication operator
 * @param V1 First vector
 * @param V2 Second vector
 * @return Element-wise product with labels from V1
 */
CMBVector operator*(const CMBVector&, const CMBVector&);

/**
 * @brief Scalar-vector multiplication operator
 * @param d Scalar value
 * @param V1 Vector
 * @return Vector with each element multiplied by scalar
 */
CMBVector operator*(double, const CMBVector&);

/**
 * @brief Vector-scalar division operator
 * @param V1 Vector
 * @param d Scalar value
 * @return Vector with each element divided by scalar
 */
CMBVector operator/(const CMBVector&, double);


#endif // CMBVector_H