#ifndef COMMANDCATALOG_H
#define COMMANDCATALOG_H

#include <QJsonObject>
#include <QString>
#include <QStringList>

#include <map>
#include <string>

/**
 * @class CommandCatalog
 * @brief The commands and arguments a script may use, read from the form definitions
 *
 * The graphical interface builds its dialogs from resources/forms_structures.json,
 * which names every command, its parameters, their types and their defaults.
 * Reading the same file here means a script is checked against exactly what the
 * forms offer, and that the two cannot drift apart.
 *
 * Filling in defaults is not a convenience. The analyses read their arguments
 * with std::map::at(), which throws when a key is absent, so a script that
 * omits a parameter the form would have supplied would otherwise abort the
 * program.
 */
class CommandCatalog
{
public:
    CommandCatalog();

    /**
     * @brief Loads the form definitions
     *
     * Searched in order: the directory given here, the SEDSAT3_RESOURCES
     * environment variable, then locations relative to the executable.
     *
     * @param explicit_directory Directory to try first, may be empty
     * @return true if the definitions were found and parsed
     */
    bool Load(const QString& explicit_directory = QString());

    /// @brief Why loading failed, if it did
    QString Error() const { return error; }

    /// @brief Directory the definitions were read from
    QString ResourceDirectory() const { return resource_directory; }

    /// @brief Whether any definitions are loaded
    bool IsLoaded() const { return !commands.isEmpty(); }

    /// @brief Every command name, sorted
    QStringList Commands() const;

    /// @brief Whether @p command is one the catalog knows
    bool HasCommand(const QString& command) const;

    /// @brief Parameter names for @p command, excluding description text
    QStringList Parameters(const QString& command) const;

    /**
     * @brief Reports anything wrong with a step
     *
     * Checks that the command exists and that every argument given is one the
     * command accepts. An unrecognised name is reported with the closest
     * accepted name, since a mistyped argument would otherwise be silently
     * ignored and the command would run with the default in its place.
     *
     * @param command Command the step names
     * @param argument_names Argument names the step supplies
     * @return One message per problem, empty when the step is usable
     */
    QStringList Problems(const QString& command, const QStringList& argument_names) const;

    /**
     * @brief Adds any parameter the script omitted, using the form's default
     * @param command Command being run
     * @param arguments Arguments to complete, modified in place
     * @return Names of the parameters that were filled in
     */
    QStringList FillDefaults(const QString& command,
                             std::map<std::string, std::string>& arguments) const;

    /// @brief Human-readable account of a command and its parameters
    QString Describe(const QString& command) const;

    /// @brief One line per command, for listing them all
    QString List() const;

private:
    QJsonObject commands;        ///< Contents of forms_structures.json
    QString resource_directory;  ///< Where it was read from
    QString error;               ///< Why loading failed
};

#endif // COMMANDCATALOG_H
