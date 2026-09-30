#ifndef SCRIPTRUNNER_H
#define SCRIPTRUNNER_H

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

#include <string>

class ConsoleHost;
class CommandCatalog;

/**
 * @class ScriptRunner
 * @brief Runs a sequence of analysis commands described by a script file
 *
 * A script is a JSON document naming a project to load and an ordered list of
 * steps. Each step is a command and its arguments, using the same command and
 * argument names the graphical interface uses, so a script expresses exactly
 * what a sequence of dialogs would.
 *
 * Steps run in order against one dataset, and several of them modify it, so
 * the order of the list is significant. That is why the steps are a JSON array
 * rather than an object.
 *
 * @code
 * {
 *   "project": "properties.cmb",
 *   "output": "run.json",
 *   "on_error": "stop",
 *   "steps": [
 *     { "command": "Outlier", "arguments": { "Threshold": "3" } },
 *     { "command": "Levenberg-Marquardt",
 *       "arguments": { "Sample": "CTAIL10", "Softmax transformation": "true" } }
 *   ]
 * }
 * @endcode
 */
class ScriptRunner
{
public:
    /**
     * @brief Constructs a runner
     * @param host Host the analyses report through (non-owning)
     * @param quiet Suppresses step narration on standard error
     */
    ScriptRunner(ConsoleHost* host, const CommandCatalog* catalog, bool quiet);

    /**
     * @brief Loads and runs a script
     *
     * Paths inside the script that are not absolute are resolved against the
     * directory holding the script, so a script and its project can be moved
     * together.
     *
     * @param script_path Path to the script file
     * @return true if every step succeeded, false if any failed or the script
     *         could not be read
     */
    bool Run(const QString& script_path);

    /// @brief Report describing the run, written after Run() returns
    QJsonObject Report() const { return report; }

    /// @brief Description of why the script could not be started, if it could not
    QString Error() const { return error; }

private:
    bool LoadProject(const QString& project_path);
    bool ValidateSteps(const QJsonArray& steps);
    QJsonObject RunStep(const QJsonObject& step, int index);
    QJsonObject RunSetupStep(const QJsonObject& step, int index, const QString& command);

    ConsoleHost* host;              ///< Non-owning, supplies warnings and progress
    const CommandCatalog* catalog;  ///< Non-owning, defines the commands
    bool quiet;          ///< Suppresses narration
    QJsonObject report;  ///< Accumulated run report
    QString error;       ///< Why the script could not be started
    QString working_folder; ///< Where analyses write files of their own
};

#endif // SCRIPTRUNNER_H
