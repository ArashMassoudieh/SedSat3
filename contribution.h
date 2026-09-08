#ifndef CONTRIBUTION_H
#define CONTRIBUTION_H

#include <string>
#include <map>
#include "interface.h"

//using namespace std;

class QFile;

/**
 * @class Contribution
 * @brief Source contribution container for Chemical Mass Balance results
 *
 * Stores source contributions as name-value pairs (map<string, double>).
 * Provides JSON serialization, file I/O, and table widget generation.
 * Used to represent the fractional or absolute contribution of each
 * identified pollution source to a receptor site.
 */
class Contribution : public map<string, double>, public Interface
{
public:
    /**
     * @brief Default constructor - creates an empty contribution set
     */
    Contribution();

    /**
     * @brief Copy constructor
     * @param rhs Contribution object to copy from
     */
    Contribution(const Contribution& rhs);

    /**
     * @brief Assignment operator
     * @param rhs Contribution object to assign from
     * @return Reference to this contribution object
     */
    Contribution& operator = (const Contribution& rhs);

    /**
     * @brief Serializes contributions to JSON format
     * @return JSON object with source names as keys and contributions as values
     */
    QJsonObject toJsonObject() const override;

    /**
     * @brief Deserializes contributions from JSON format
     * @param jsonobject JSON object to read from
     * @return true if successful, false otherwise
     */
    bool ReadFromJsonObject(const QJsonObject& jsonobject) override;

    /**
     * @brief Converts contributions to colon-separated string format
     *
     * Format: "source_name:contribution_value\n" for each source
     *
     * @return String representation with one source per line
     */
    string ToString() const override;

    /**
     * @brief Writes contributions to file
     * @param file File pointer to write to
     * @return true if successful
     */
    bool writetofile(QFile*) override;

    /**
     * @brief Reads contributions from string list
     *
     * Expected format: Each string is "source_name:contribution_value"
     *
     * @param strlist List of colon-separated source:value strings
     * @return true if successful
     */
    bool Read(const QStringList& strlist) override;

    /**
     * @brief Creates a QTableWidget for displaying contributions
     *
     * Generates a read-only single-column table with source names as
     * row labels and contribution values in the column.
     *
     * @return Pointer to newly created QTableWidget (caller takes ownership)
     */
    QTableWidget* ToTable() override;
};

#endif // CONTRIBUTION_H