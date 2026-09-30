#ifndef SETUPCOMMANDS_H
#define SETUPCOMMANDS_H

#include <QJsonObject>
#include <QString>
#include <QStringList>

class SourceSinkData;

/**
 * @namespace SetupCommands
 * @brief Commands that configure a dataset rather than analyse it
 *
 * A project file records which elements and samples take part in an analysis,
 * what role each constituent plays and which group is the target. In the
 * graphical application those are set in tables and dialogs, so none of them
 * is reachable through Conductor::Execute and a script could otherwise only
 * re-run the configuration the project was last saved with.
 *
 * These commands change that configuration before the analyses run. Where a
 * setup command and the project file disagree, the command wins: the script is
 * the more specific instruction, and running one is how a script says it wants
 * something other than what was saved.
 *
 * Commands:
 * - `set-target-group`    `{ "group": "Target" }`
 * - `set-elements`        `{ "include": [...], "exclude": [...], "only": [...] }`
 * - `set-samples`         `{ "group": "Bank", "include": [...], "exclude": [...] }`
 * - `set-element-role`    `{ "element": "C13", "role": "Isotope",
 *                            "base element": "TOC", "standard ratio": "0.011113" }`
 */
namespace SetupCommands
{
    /// @brief Whether @p command is one of the setup commands
    bool IsSetupCommand(const QString& command);

    /// @brief Every setup command name
    QStringList Names();

    /// @brief Human-readable account of a setup command and its arguments
    QString Describe(const QString& command);

    /**
     * @brief Applies a setup command to a dataset
     *
     * @param command Name of the setup command
     * @param arguments Arguments as given in the script
     * @param data Dataset to modify
     * @param changes Filled with a line describing each change made
     * @param error Filled with the reason when the command could not be applied
     * @return true when the command was applied
     */
    bool Apply(const QString& command,
               const QJsonObject& arguments,
               SourceSinkData* data,
               QStringList& changes,
               QString& error);
}

#endif // SETUPCOMMANDS_H
