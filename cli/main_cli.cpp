#include "consolehost.h"
#include "scriptrunner.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QStringList>

#include <iostream>

namespace {

void PrintUsage()
{
    std::cout
        << "sedsat3-cli - run SedSAT3 analyses from a script\n"
           "\n"
           "Usage:\n"
           "  sedsat3-cli <script.json> [--output <file>] [--quiet]\n"
           "  sedsat3-cli --help\n"
           "\n"
           "A script names a project to load and an ordered list of steps. Each step\n"
           "is a command with its arguments, using the same names the graphical\n"
           "interface uses. Steps run in order against one dataset; several of them\n"
           "modify it, so the order matters.\n"
           "\n"
           "  {\n"
           "    \"project\": \"properties.cmb\",\n"
           "    \"output\": \"run.json\",\n"
           "    \"on_error\": \"stop\",\n"
           "    \"steps\": [\n"
           "      { \"command\": \"Outlier\", \"arguments\": { \"Threshold\": \"3\" } }\n"
           "    ]\n"
           "  }\n"
           "\n"
           "Options:\n"
           "  --output <file>  Where to write the run report. Overrides the script's\n"
           "                   \"output\", and defaults to standard output.\n"
           "  --quiet          Suppress progress and step narration on standard error.\n"
           "                   Warnings are still reported.\n"
           "  --help           Show this message.\n"
           "\n"
           "Relative paths in the script are resolved against the script's directory.\n"
           "Exit status is 0 when every step succeeded, 1 otherwise.\n";
}

} // namespace

int main(int argc, char* argv[])
{
    // QCoreApplication rather than QApplication: this program creates no
    // widgets, and a console run should not require a display.
    QCoreApplication application(argc, argv);

    QString script_path;
    QString output_path;
    bool quiet = false;

    const QStringList arguments = QCoreApplication::arguments();
    for (int i = 1; i < arguments.size(); i++)
    {
        const QString argument = arguments[i];

        if (argument == "--help" || argument == "-h")
        {
            PrintUsage();
            return 0;
        }
        else if (argument == "--quiet" || argument == "-q")
        {
            quiet = true;
        }
        else if (argument == "--output" || argument == "-o")
        {
            if (i + 1 >= arguments.size())
            {
                std::cerr << "error: --output needs a file name" << std::endl;
                return 1;
            }
            output_path = arguments[++i];
        }
        else if (argument.startsWith("-"))
        {
            std::cerr << "error: unknown option '" << argument.toStdString() << "'" << std::endl;
            std::cerr << "Run with --help for usage." << std::endl;
            return 1;
        }
        else if (script_path.isEmpty())
        {
            script_path = argument;
        }
        else
        {
            std::cerr << "error: more than one script given" << std::endl;
            return 1;
        }
    }

    if (script_path.isEmpty())
    {
        PrintUsage();
        return 1;
    }

    ConsoleHost host(quiet);
    ScriptRunner runner(&host, quiet);

    const bool succeeded = runner.Run(script_path);

    if (!runner.Error().isEmpty())
    {
        std::cerr << "error: " << runner.Error().toStdString() << std::endl;
        return 1;
    }

    // The script may name its own output file; an explicit --output wins.
    if (output_path.isEmpty())
    {
        QFile script_file(script_path);
        if (script_file.open(QIODevice::ReadOnly))
        {
            const QJsonObject script =
                QJsonDocument::fromJson(script_file.readAll()).object();
            script_file.close();

            const QString from_script = script["output"].toString();
            if (!from_script.isEmpty())
            {
                QFileInfo info(from_script);
                output_path = info.isAbsolute()
                    ? from_script
                    : QFileInfo(script_path).absoluteDir().absoluteFilePath(from_script);
            }
        }
    }

    const QByteArray report =
        QJsonDocument(runner.Report()).toJson(QJsonDocument::Indented);

    if (output_path.isEmpty())
    {
        std::cout << report.constData();
    }
    else
    {
        QFile output(output_path);
        if (!output.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            std::cerr << "error: could not write report to '"
                      << output_path.toStdString() << "': "
                      << output.errorString().toStdString() << std::endl;
            return 1;
        }
        output.write(report);
        output.close();

        if (!quiet)
            std::cerr << "report written to " << output_path.toStdString() << std::endl;
    }

    return succeeded ? 0 : 1;
}
