#include "commandcatalog.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QProcessEnvironment>

namespace {

const char* const kFormsFile = "forms_structures.json";

/// @brief "Description" entries are help text in the forms, not parameters
bool IsDescription(const QJsonObject& parameter)
{
    return parameter["type"].toString() == "Description";
}

/// @brief Levenshtein distance, used to suggest a name for a mistyped argument
int EditDistance(const QString& a, const QString& b)
{
    const int n = a.size();
    const int m = b.size();
    if (n == 0) return m;
    if (m == 0) return n;

    QVector<int> previous(m + 1);
    QVector<int> current(m + 1);
    for (int j = 0; j <= m; j++)
        previous[j] = j;

    for (int i = 1; i <= n; i++)
    {
        current[0] = i;
        for (int j = 1; j <= m; j++)
        {
            const int cost = (a[i - 1].toLower() == b[j - 1].toLower()) ? 0 : 1;
            current[j] = qMin(qMin(current[j - 1] + 1, previous[j] + 1),
                              previous[j - 1] + cost);
        }
        previous = current;
    }
    return previous[m];
}

} // namespace

CommandCatalog::CommandCatalog()
{
}

bool CommandCatalog::Load(const QString& explicit_directory)
{
    QStringList candidates;

    if (!explicit_directory.isEmpty())
        candidates << explicit_directory;

    const QString from_environment =
        QProcessEnvironment::systemEnvironment().value("SEDSAT3_RESOURCES");
    if (!from_environment.isEmpty())
        candidates << from_environment;

    // The graphical application looks two levels up from its build directory.
    // A command line build may sit anywhere, so try the plausible places
    // rather than assuming one layout.
    const QString application_directory = QCoreApplication::applicationDirPath();
    candidates << application_directory + "/resources"
               << application_directory + "/../resources"
               << application_directory + "/../../resources"
               << application_directory + "/../Resources"
               << QDir::currentPath() + "/resources";

    QStringList tried;
    for (const QString& directory : candidates)
    {
        const QString path = QDir(directory).absoluteFilePath(kFormsFile);
        tried << path;

        QFile file(path);
        if (!file.open(QIODevice::ReadOnly))
            continue;

        const QByteArray contents = file.readAll();
        file.close();

        QJsonParseError parse_error;
        const QJsonDocument document = QJsonDocument::fromJson(contents, &parse_error);
        if (document.isNull() || !document.isObject())
        {
            error = "Found " + path + " but could not parse it: " + parse_error.errorString();
            return false;
        }

        commands = document.object();
        resource_directory = QDir(directory).absolutePath();
        return true;
    }

    error = "Could not find " + QString(kFormsFile) +
            ". Give --resources <directory>, or set SEDSAT3_RESOURCES. Looked in:\n  " +
            tried.join("\n  ");
    return false;
}

QStringList CommandCatalog::Commands() const
{
    QStringList names = commands.keys();
    names.sort(Qt::CaseInsensitive);
    return names;
}

bool CommandCatalog::HasCommand(const QString& command) const
{
    return commands.contains(command);
}

QStringList CommandCatalog::Parameters(const QString& command) const
{
    QStringList names;
    const QJsonObject definition = commands[command].toObject();

    for (const QString& key : definition.keys())
    {
        if (!IsDescription(definition[key].toObject()))
            names << key;
    }
    return names;
}

QStringList CommandCatalog::Problems(const QString& command,
                                     const QStringList& argument_names) const
{
    QStringList problems;

    if (!HasCommand(command))
    {
        QString message = "Unknown command '" + command + "'.";

        // Suggest a command rather than only rejecting the step.
        QString best;
        int best_distance = 1000;
        for (const QString& known : Commands())
        {
            const int distance = EditDistance(command, known);
            if (distance < best_distance)
            {
                best_distance = distance;
                best = known;
            }
        }
        if (best_distance <= 5 && !best.isEmpty())
            message += " Did you mean '" + best + "'?";

        problems << message;
        return problems;
    }

    const QStringList accepted = Parameters(command);
    for (const QString& given : argument_names)
    {
        if (accepted.contains(given))
            continue;

        QString message = "Command '" + command + "' has no argument '" + given + "'.";

        QString best;
        int best_distance = 1000;
        for (const QString& known : accepted)
        {
            const int distance = EditDistance(given, known);
            if (distance < best_distance)
            {
                best_distance = distance;
                best = known;
            }
        }
        if (best_distance <= 6 && !best.isEmpty())
            message += " Did you mean '" + best + "'?";

        problems << message;
    }

    return problems;
}

QStringList CommandCatalog::FillDefaults(const QString& command,
                                         std::map<std::string, std::string>& arguments) const
{
    QStringList filled;

    if (!HasCommand(command))
        return filled;

    const QJsonObject definition = commands[command].toObject();

    for (const QString& key : definition.keys())
    {
        const QJsonObject parameter = definition[key].toObject();
        if (IsDescription(parameter))
            continue;

        if (arguments.count(key.toStdString()) > 0)
            continue;

        // An absent default becomes an empty string, which is what an
        // untouched field in the form would have produced.
        const QString value = parameter["default"].toString();
        arguments[key.toStdString()] = value.toStdString();
        filled << key;
    }

    return filled;
}

QString CommandCatalog::Describe(const QString& command) const
{
    if (!HasCommand(command))
        return "Unknown command '" + command + "'.\n";

    const QJsonObject definition = commands[command].toObject();
    QString text = command + "\n";

    for (const QString& key : definition.keys())
    {
        const QJsonObject parameter = definition[key].toObject();
        if (IsDescription(parameter))
        {
            const QString description = parameter["default"].toString();
            if (!description.isEmpty())
                text += "\n  " + description + "\n";
        }
    }

    text += "\nArguments:\n";
    bool any = false;
    for (const QString& key : definition.keys())
    {
        const QJsonObject parameter = definition[key].toObject();
        if (IsDescription(parameter))
            continue;

        any = true;
        QString line = "  " + key;
        line = line.leftJustified(52, ' ');
        line += parameter["type"].toString();

        const QString source = parameter["source"].toString();
        if (!source.isEmpty())
        {
            // Sources beginning with "Items:" list their choices; the rest
            // are filled from the loaded project.
            if (source.startsWith("Items:"))
                line += "  one of: " + source.mid(6);
            else
                line += "  from the project (" + source + ")";
        }

        if (parameter.contains("default"))
            line += "  default: \"" + parameter["default"].toString() + "\"";

        text += line + "\n";
    }

    if (!any)
        text += "  (none)\n";

    return text;
}

QString CommandCatalog::List() const
{
    QString text;
    for (const QString& command : Commands())
    {
        QString line = "  " + command;
        line = line.leftJustified(40, ' ');
        line += QString::number(Parameters(command).size()) + " argument(s)";
        text += line + "\n";
    }
    return text;
}
