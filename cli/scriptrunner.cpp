#include "scriptrunner.h"
#include "consolehost.h"

#include "conductor.h"
#include "sourcesinkdata.h"
#include "results.h"

#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
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

ScriptRunner::ScriptRunner(ConsoleHost* _host, bool _quiet)
    : host(_host),
      quiet(_quiet)
{
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
    step_report["arguments"] = echoed_arguments;

    if (!quiet)
        std::cerr << "[" << index + 1 << "] " << command.toStdString() << std::endl;

    host->ClearWarnings();

    Conductor conductor(host);
    conductor.SetData(&script_data);

    QElapsedTimer timer;
    timer.start();
    const bool succeeded = conductor.Execute(command.toStdString(), arguments);
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
    report["started"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    const QJsonArray steps = script["steps"].toArray();
    QJsonArray step_reports;
    bool all_succeeded = true;

    for (int i = 0; i < steps.size(); i++)
    {
        const QJsonObject step_report = RunStep(steps[i].toObject(), i);
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
