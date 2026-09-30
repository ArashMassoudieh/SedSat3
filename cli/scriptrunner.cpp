#include "scriptrunner.h"
#include "consolehost.h"
#include "commandcatalog.h"
#include "setupcommands.h"

#include "conductor.h"
#include "sourcesinkdata.h"
#include "results.h"

#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <stdexcept>
#include <QJsonDocument>

#include <iostream>
#include <map>
#include <memory>

namespace {

/// @brief Resolves @p path against @p base_dir unless it is already absolute
QString ResolveAgainst(const QString& base_dir, const QString& path)
{
    if (path.isEmpty())
        return path;

    QFileInfo info(path);
    if (info.isAbsolute())
        return path;

    return QDir(base_dir).absoluteFilePath(path);
}

} // namespace

// The dataset and the conductor live for the whole run, because steps modify
// the dataset and later steps read what earlier ones changed.
static SourceSinkData script_data;

ScriptRunner::ScriptRunner(ConsoleHost* _host, const CommandCatalog* _catalog, bool _quiet)
    : host(_host),
      catalog(_catalog),
      quiet(_quiet)
{
}

bool ScriptRunner::ValidateSteps(const QJsonArray& steps)
{
    // Every step is checked before any of them runs. A script is a pipeline
    // whose later steps read what earlier ones changed, so stopping halfway
    // through because of a typo in the last step would leave the project in a
    // state nobody asked for.
    QStringList problems;

    for (int i = 0; i < steps.size(); i++)
    {
        const QJsonObject step = steps[i].toObject();
        const QString command = step["command"].toString();
        const QString where = "step " + QString::number(i + 1) + ": ";

        if (command.isEmpty())
        {
            problems << where + "no \"command\" given.";
            continue;
        }

        if (SetupCommands::IsSetupCommand(command))
            continue;

        if (catalog == nullptr || !catalog->IsLoaded())
            continue;

        for (const QString& problem :
             catalog->Problems(command, step["arguments"].toObject().keys()))
        {
            problems << where + problem;
        }
    }

    if (problems.isEmpty())
        return true;

    error = "The script was not run because of the following:\n  " + problems.join("\n  ");
    return false;
}

QJsonObject ScriptRunner::RunSetupStep(const QJsonObject& step, int index, const QString& command)
{
    QJsonObject step_report;
    step_report["step"] = index + 1;
    step_report["command"] = command;
    step_report["arguments"] = step["arguments"].toObject();

    if (!quiet)
        std::cerr << "[" << index + 1 << "] " << command.toStdString() << std::endl;

    QStringList changes;
    QString setup_error;

    const bool applied = SetupCommands::Apply(command,
                                              step["arguments"].toObject(),
                                              &script_data,
                                              changes,
                                              setup_error);

    if (!applied)
    {
        step_report["status"] = "failed";
        step_report["error"] = setup_error;
        if (!quiet)
            std::cerr << "  error: " << setup_error.toStdString() << std::endl;
        return step_report;
    }

    step_report["status"] = "succeeded";

    QJsonArray change_array;
    for (const QString& change : changes)
    {
        change_array.append(change);
        if (!quiet)
            std::cerr << "  " << change.toStdString() << std::endl;
    }
    step_report["changes"] = change_array;

    return step_report;
}

bool ScriptRunner::LoadProject(const QString& project_path)
{
    QFile file(project_path);

    if (!file.open(QIODevice::ReadOnly))
    {
        error = "Could not open project '" + project_path + "': " + file.errorString();
        return false;
    }

    const bool loaded = script_data.ReadFromFile(&file);
    file.close();

    if (!loaded)
    {
        error = "Could not read project '" + project_path + "'. It may not be a SedSAT project file.";
        return false;
    }

    // Reading the file is only half of loading a project. Distribution
    // parameters are derived from the data rather than stored, and the
    // analyses read them, so they have to be built here exactly as
    // MainWindow::LoadModel builds them after reading the same file.
    script_data.PopulateElementDistributions();
    script_data.AssignAllDistributions();

    return true;
}

QJsonObject ScriptRunner::RunStep(const QJsonObject& step, int index)
{
    QJsonObject step_report;

    const QString command = step["command"].toString();
    step_report["step"] = index + 1;
    step_report["command"] = command;

    if (command.isEmpty())
    {
        step_report["status"] = "failed";
        step_report["error"] = "Step has no \"command\".";
        return step_report;
    }

    // Argument names are the labels the graphical forms use, so a script says
    // the same thing a dialog would.
    std::map<std::string, std::string> arguments;
    const QJsonObject argument_object = step["arguments"].toObject();
    QJsonObject echoed_arguments;

    for (const QString& key : argument_object.keys())
    {
        const QJsonValue value = argument_object[key];
        const QString as_text = value.isString() ? value.toString()
                              : value.isBool()   ? (value.toBool() ? "true" : "false")
                              : value.isDouble() ? QString::number(value.toDouble())
                                                 : QString();
        arguments[key.toStdString()] = as_text.toStdString();
        echoed_arguments[key] = as_text;
    }
    // The analyses read their arguments with std::map::at(), which throws when
    // a key is absent. The form would always have supplied every field, so
    // anything the script left out is filled from the form's default.
    if (catalog != nullptr && catalog->IsLoaded())
    {
        const QStringList filled = catalog->FillDefaults(command, arguments);
        if (!filled.isEmpty())
        {
            QJsonArray defaulted;
            for (const QString& name : filled)
                defaulted.append(name);
            step_report["defaulted"] = defaulted;
        }
    }

    step_report["arguments"] = echoed_arguments;

    if (!quiet)
        std::cerr << "[" << index + 1 << "] " << command.toStdString() << std::endl;

    host->ClearWarnings();

    Conductor conductor(host);
    conductor.SetData(&script_data);

    // Analyses that write files of their own resolve the name against the
    // working folder. The graphical application always sets one; without it
    // the name resolves to the root of the filesystem and the write fails.
    conductor.SetWorkingFolder(working_folder);

    QElapsedTimer timer;
    timer.start();

    bool succeeded = false;
    try
    {
        succeeded = conductor.Execute(command.toStdString(), arguments);
    }
    catch (const std::exception& exception)
    {
        step_report["status"] = "failed";
        step_report["error"] = QString("The analysis stopped with an error: ") + exception.what();
        step_report["seconds"] = timer.elapsed() / 1000.0;
        return step_report;
    }

    step_report["seconds"] = timer.elapsed() / 1000.0;

    // Warnings explain why a command declined to run, so they belong in the
    // report rather than only on the terminal.
    const std::vector<std::string>& warnings = host->Warnings();
    if (!warnings.empty())
    {
        QJsonArray warning_array;
        for (const std::string& warning : warnings)
            warning_array.append(QString::fromStdString(warning));
        step_report["warnings"] = warning_array;
    }

    if (!succeeded)
    {
        step_report["status"] = "failed";
        return step_report;
    }

    step_report["status"] = "succeeded";

    std::unique_ptr<Results> results(conductor.GetResults());
    if (results)
    {
        step_report["name"] = QString::fromStdString(results->GetName());
        if (!results->Error().empty())
            step_report["error"] = QString::fromStdString(results->Error());
        step_report["results"] = results->toJsonObject();
    }

    return step_report;
}

bool ScriptRunner::Run(const QString& script_path)
{
    QFile script_file(script_path);
    if (!script_file.open(QIODevice::ReadOnly))
    {
        error = "Could not open script '" + script_path + "': " + script_file.errorString();
        return false;
    }

    const QByteArray script_text = script_file.readAll();
    script_file.close();

    QJsonParseError parse_error;
    const QJsonDocument document = QJsonDocument::fromJson(script_text, &parse_error);
    if (document.isNull())
    {
        error = "Script is not valid JSON: " + parse_error.errorString() +
                " (offset " + QString::number(parse_error.offset) + ")";
        return false;
    }
    if (!document.isObject())
    {
        error = "Script must be a JSON object with \"project\" and \"steps\".";
        return false;
    }

    const QJsonObject script = document.object();
    const QString script_dir = QFileInfo(script_path).absolutePath();

    if (!script.contains("steps") || !script["steps"].isArray())
    {
        error = "Script has no \"steps\" array. Steps must be an array, because they run in order.";
        return false;
    }

    // Files an analysis writes land beside the script unless the script says
    // otherwise, which keeps a run self-contained and predictable.
    working_folder = script.contains("working_folder")
        ? ResolveAgainst(script_dir, script["working_folder"].toString())
        : script_dir;
    QDir().mkpath(working_folder);

    const QString project_path = ResolveAgainst(script_dir, script["project"].toString());
    if (project_path.isEmpty())
    {
        error = "Script has no \"project\" to load.";
        return false;
    }

    if (!LoadProject(project_path))
        return false;

    const bool stop_on_error = (script["on_error"].toString("stop") != "continue");

    report["script"] = QFileInfo(script_path).absoluteFilePath();
    report["project"] = project_path;
    report["working_folder"] = working_folder;
    report["started"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    const QJsonArray steps = script["steps"].toArray();

    if (!ValidateSteps(steps))
        return false;

    QJsonArray step_reports;
    bool all_succeeded = true;

    for (int i = 0; i < steps.size(); i++)
    {
        const QJsonObject step = steps[i].toObject();
        const QString step_command = step["command"].toString();

        const QJsonObject step_report =
            SetupCommands::IsSetupCommand(step_command)
                ? RunSetupStep(step, i, step_command)
                : RunStep(step, i);
        step_reports.append(step_report);

        if (step_report["status"].toString() != "succeeded")
        {
            all_succeeded = false;
            if (stop_on_error)
            {
                report["stopped_at_step"] = i + 1;
                break;
            }
        }
    }

    report["steps"] = step_reports;
    report["finished"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    report["succeeded"] = all_succeeded;

    return all_succeeded;
}
