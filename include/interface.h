#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <QStringList>
#include <qjsonobject.h>
#include <qtablewidget.h>
#include "parameter.h"

class QFile;

/**
 * @enum options_key
 * @brief Configuration option identifiers for Interface-derived classes
 *
 * Enumeration of available configuration options that can be set on Interface objects.
 */
enum class options_key {
    single_column_x  ///< Enable single-column mode for X-axis data representation
};

/**
 * @struct options
 * @brief Configuration options for data presentation and serialization
 *
 * Contains settings that control how data is displayed, exported, and interpreted
 * in Interface-derived classes. These options affect serialization format and
 * visual representation.
 */
struct options
{
    bool single_column_x = false;      ///< If true, X values are stored in a single column during serialization
    QString X_suffix = "x-value";      ///< Suffix appended to X-axis field names in serialization
    QString Y_suffix = "y-value";      ///< Suffix appended to Y-axis field names in serialization
};

/**
 * @class Interface
 * @brief Abstract base class providing common serialization and visualization interface
 *
 * Interface serves as an abstract base class for data structures that need to be:
 * - Serialized to/from various formats (JSON, strings, files)
 * - Displayed in Qt table widgets
 * - Annotated with metadata (notes, axis labels)
 * - Configured with display options (highlighting, limits)
 *
 * This class provides a unified interface for data handling across the application,
 * ensuring consistent behavior for file I/O, JSON serialization, and Qt integration.
 *
 * Derived classes must implement the virtual methods to provide specific serialization
 * and visualization behavior appropriate to their data structure.
 *
 * @note This is an abstract base class. Concrete implementations include:
 *       - Elemental_Profile: Single sample's elemental concentrations
 *       - Elemental_Profile_Set: Collection of samples
 *       - CMBVector: Vector data with labels
 *       - CMBMatrix: Matrix data with row/column labels
 *
 * @see Elemental_Profile
 * @see Elemental_Profile_Set
 * @see CMBVector
 * @see CMBMatrix
 */
class Interface
{
public:
    /**
     * @brief Default constructor
     *
     * Initializes an Interface object with default settings:
     * - Empty notes
     * - Default axis labels ("Sample", "Value")
     * - Highlight limits disabled
     * - Default serialization options
     */
    Interface();

    /**
     * @brief Copy constructor
     * @param intf The Interface object to copy from
     *
     * Creates a deep copy of another Interface object, including all configuration
     * options, axis labels, notes, and limit settings.
     */
    Interface(const Interface &intf);

    /**
     * @brief Assignment operator
     * @param intf The Interface object to copy from
     * @return Reference to this object for chaining
     *
     * Copies all data and settings from another Interface object.
     */
    Interface& operator=(const Interface &intf);

    /**
     * @brief Convert object to string representation
     * @return String representation of the object's data
     *
     * Pure virtual method that must be implemented by derived classes to provide
     * a human-readable string representation of their data. Typically used for
     * text file export or debugging.
     *
     * @note Format is determined by the derived class implementation
     */
    virtual string ToString() const;

    /**
     * @brief Serialize object to JSON format
     * @return QJsonObject containing the serialized data
     *
     * Pure virtual method for JSON serialization. Derived classes must implement
     * this to convert their data structure to a JSON object suitable for saving
     * to .json files or transmitting over network.
     *
     * @note The JSON structure is determined by the derived class
     */
    virtual QJsonObject toJsonObject() const;

    /**
     * @brief Deserialize object from JSON format
     * @param jsonobject The JSON object to read from
     * @return true if deserialization was successful, false otherwise
     *
     * Pure virtual method for JSON deserialization. Derived classes must implement
     * this to reconstruct their data structure from a JSON object.
     *
     * @warning Implementations should validate JSON structure and handle missing fields gracefully
     */
    virtual bool ReadFromJsonObject(const QJsonObject &jsonobject);

    /**
     * @brief Write object data to a file
     * @param file Pointer to an open QFile object for writing
     * @return true if write was successful, false otherwise
     *
     * Pure virtual method for file output. Derived classes implement this to write
     * their data to an open file stream in their preferred text format.
     *
     * @pre file must be opened in write mode
     * @note File is not closed by this method
     */
    virtual bool writetofile(QFile* file);

    /**
     * @brief Parse object data from a string list
     * @param strlist List of strings containing the data to parse
     * @return true if parsing was successful, false otherwise
     *
     * Pure virtual method for text-based deserialization. Each string in the list
     * typically represents one line of input data.
     *
     * @note Format and parsing rules are determined by the derived class
     */
    virtual bool Read(const QStringList &strlist);

    /**
     * @brief Create a Qt table widget representation of the data
     * @return Pointer to a newly allocated QTableWidget displaying the data
     *
     * Pure virtual method that creates a visual table representation of the object's
     * data suitable for display in the Qt GUI. The returned widget is dynamically
     * allocated and should be managed by Qt's parent-child ownership system.
     *
     * @note Caller does not need to delete the returned widget if added to a parent
     * @note Highlighting based on limit settings is applied if enabled
     */
    virtual QTableWidget *ToTable();

    /**
     * @brief Get the notes/annotations associated with this object
     * @return String containing all notes, semicolon-separated if multiple
     *
     * Notes are used to store warnings, outlier information, QA/QC flags,
     * or other metadata about the data.
     */
    string GetNotes() const {
        return Notes;
    }

    /**
     * @brief Set the notes for this object, replacing any existing notes
     * @param note The new note text
     */
    void SetNotes(const string &note) {
        Notes = note;
    }

    /**
     * @brief Append a note to existing notes
     * @param note The note text to append
     *
     * If notes are empty, sets the note directly. Otherwise, appends with
     * semicolon separator ("; ") for readability.
     */
    void AppendtoNotes(const string &note)
    {
        if (Notes == "")
            Notes += note;
        else
            Notes += "; " + note;
    }

    /**
     * @brief Clear all notes associated with this object
     */
    void ClearNotes() {
        Notes = "";
    }

    double lowlimit;   ///< Lower threshold for value highlighting (when enabled)
    double highlimit;  ///< Upper threshold for value highlighting (when enabled)

    /**
     * @brief Flag indicating whether to highlight values outside defined limits
     *
     * When true, the ToTable() method will highlight cells with values outside
     * the range [lowlimit, highlimit] in red, useful for identifying outliers
     * or quality control issues.
     */
    bool highlightoutsideoflimit = false;

    /**
     * @brief Set upper or lower limit for value highlighting
     * @param lowhigh Specifies whether setting high or low limit
     * @param value The threshold value
     *
     * Sets the specified limit and automatically enables highlighting.
     * Values outside [lowlimit, highlimit] will be highlighted when displayed.
     *
     * @see highlightoutsideoflimit
     */
    void SetLimit(_range lowhigh, const double &value)
    {
        if (lowhigh == _range::high)
            highlimit = value;
        else
            lowlimit = value;
        highlightoutsideoflimit = true;
    }

    /**
     * @brief Set a configuration option
     * @param opt The option identifier to set
     * @param val The boolean value to set
     *
     * Currently supports:
     * - options_key::single_column_x: Enable single-column mode for X values
     */
    void SetOption(options_key opt, bool val)
    {
        if (opt == options_key::single_column_x)
            Options.single_column_x = val;
    }

    /**
     * @brief Get the current value of a configuration option
     * @param opt The option identifier to query
     * @return The current boolean value of the option, false if option not recognized
     */
    bool Option(options_key opt)
    {
        if (opt == options_key::single_column_x)
            return Options.single_column_x;
        return false;
    }

    /**
     * @brief Get reference to the options structure
     * @return Reference to the internal options object
     *
     * Provides direct access to all configuration options for batch modification
     * or inspection.
     */
    options& GetOptions()
    {
        return Options;
    }

    /**
     * @brief Get the X-axis label
     * @return The current X-axis label string
     *
     * X-axis labels are used in plots and table headers to identify the
     * independent variable or sample identifier.
     */
    QString XAxisLabel() const {
        return XAxis_Label;
    }

    /**
     * @brief Get the Y-axis label
     * @return The current Y-axis label string
     *
     * Y-axis labels are used in plots and table headers to identify the
     * dependent variable or measurement type (e.g., "Concentration (mg/kg)").
     */
    QString YAxisLabel() const {
        return YAxis_Label;
    }

    /**
     * @brief Set the X-axis label
     * @param label The new X-axis label
     * @return true (always succeeds)
     */
    bool SetXAxisLabel(const QString &label) {
        XAxis_Label = label;
        return true;
    }

    /**
     * @brief Set the Y-axis label
     * @param label The new Y-axis label
     * @return true (always succeeds)
     */
    bool SetYAxisLabel(const QString &label)
    {
        YAxis_Label = label;
        return true;
    }

private:
    /**
     * @brief Textual annotations and metadata about the data
     *
     * Stores semicolon-separated notes such as outlier warnings, QA/QC flags,
     * or processing history. Can be displayed to users or included in reports.
     */
    string Notes;

    /**
     * @brief Configuration options for serialization and display
     *
     * Contains settings that control how data is formatted during export
     * and how it should be interpreted during import.
     */
    options Options;

    /**
     * @brief Label for the X-axis in plots and table headers
     *
     * Default is "Sample" but can be customized to reflect the actual
     * independent variable (e.g., "Time", "Distance", "Sample ID").
     */
    QString XAxis_Label = "Sample";

    /**
     * @brief Label for the Y-axis in plots and table headers
     *
     * Default is "Value" but should be customized to reflect the measurement
     * type and units (e.g., "Lead Concentration (mg/kg)", "δ15N (‰)").
     */
    QString YAxis_Label = "Value";
};

#endif // INTERFACE_H
