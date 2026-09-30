#include "setupcommands.h"

#include "sourcesinkdata.h"
#include "elemental_profile.h"
#include "elemental_profile_set.h"

#include <QJsonArray>

namespace {

/// @brief Reads a string array argument, also accepting a single string
QStringList StringList(const QJsonObject& arguments, const QString& key)
{
    QStringList values;
    const QJsonValue value = arguments[key];

    if (value.isString())
    {
        values << value.toString();
    }
    else if (value.isArray())
    {
        for (const QJsonValue& entry : value.toArray())
        {
            if (entry.isString())
                values << entry.toString();
        }
    }
    return values;
}

/// @brief Sets whether one constituent takes part, reporting an unknown name
bool SetElementIncluded(SourceSinkData* data,
                        const QString& element,
                        bool included,
                        QStringList& changes,
                        QString& error)
{
    element_information* information = data->GetElementInformation(element.toStdString());
    if (information == nullptr)
    {
        error = "No constituent named '" + element + "' in this project.";
        return false;
    }

    if (information->include_in_analysis != included)
    {
        information->include_in_analysis = included;
        changes << (included ? "included '" + element + "'" : "excluded '" + element + "'");
    }
    return true;
}

} // namespace

namespace SetupCommands {

QStringList Names()
{
    return QStringList() << "set-target-group"
                         << "set-elements"
                         << "set-samples"
                         << "set-element-role";
}

bool IsSetupCommand(const QString& command)
{
    return Names().contains(command);
}

QString Describe(const QString& command)
{
    if (command == "set-target-group")
        return "set-target-group\n"
               "\n  Chooses which group holds the target samples.\n"
               "\nArguments:\n"
               "  group                   name of the group\n";

    if (command == "set-elements")
        return "set-elements\n"
               "\n  Chooses which constituents take part in the analyses.\n"
               "\nArguments:\n"
               "  only                    include exactly these and exclude all others\n"
               "  include                 include these, leaving the rest as they are\n"
               "  exclude                 exclude these, leaving the rest as they are\n"
               "\n  Each takes a name or a list of names. \"only\" is applied first.\n";

    if (command == "set-samples")
        return "set-samples\n"
               "\n  Chooses which samples take part in the analyses.\n"
               "\nArguments:\n"
               "  group                   group the samples belong to (required)\n"
               "  only                    include exactly these and exclude all others\n"
               "  include                 include these\n"
               "  exclude                 exclude these\n";

    if (command == "set-element-role")
        return "set-element-role\n"
               "\n  Sets what a constituent is and, for an isotope, what it is measured\n"
               "  against.\n"
               "\nArguments:\n"
               "  element                 name of the constituent (required)\n"
               "  role                    Element, Isotope, ParticleSize, OM or DoNotInclude\n"
               "  base element            for an isotope, the constituent supplying its\n"
               "                          concentration\n"
               "  standard ratio          for an isotope, the standard isotopic ratio\n";

    return "Unknown setup command '" + command + "'.\n";
}

bool Apply(const QString& command,
           const QJsonObject& arguments,
           SourceSinkData* data,
           QStringList& changes,
           QString& error)
{
    if (data == nullptr)
    {
        error = "No project is loaded.";
        return false;
    }

    // ---------------------------------------------------------------- target
    if (command == "set-target-group")
    {
        const QString group = arguments["group"].toString();
        if (group.isEmpty())
        {
            error = "set-target-group needs a \"group\".";
            return false;
        }
        if (data->count(group.toStdString()) == 0)
        {
            error = "No group named '" + group + "' in this project.";
            return false;
        }
        if (!data->SetTargetGroup(group.toStdString()))
        {
            error = "Could not set '" + group + "' as the target group.";
            return false;
        }
        changes << "target group is '" + group + "'";
        return true;
    }

    // -------------------------------------------------------------- elements
    if (command == "set-elements")
    {
        const QStringList only = StringList(arguments, "only");
        const QStringList include = StringList(arguments, "include");
        const QStringList exclude = StringList(arguments, "exclude");

        if (only.isEmpty() && include.isEmpty() && exclude.isEmpty())
        {
            error = "set-elements needs \"only\", \"include\" or \"exclude\".";
            return false;
        }

        if (!only.isEmpty())
        {
            // Check every name before changing anything, so a typo does not
            // leave the project half configured.
            for (const QString& element : only)
            {
                if (data->GetElementInformation(element.toStdString()) == nullptr)
                {
                    error = "No constituent named '" + element + "' in this project.";
                    return false;
                }
            }

            const vector<string> all = data->GetElementNames();
            for (const string& name : all)
            {
                const QString element = QString::fromStdString(name);
                if (!SetElementIncluded(data, element, only.contains(element), changes, error))
                    return false;
            }
        }

        for (const QString& element : include)
        {
            if (!SetElementIncluded(data, element, true, changes, error))
                return false;
        }
        for (const QString& element : exclude)
        {
            if (!SetElementIncluded(data, element, false, changes, error))
                return false;
        }

        // The orderings are derived from the inclusion flags, so rebuild them.
        data->PopulateConstituentOrders();
        return true;
    }

    // --------------------------------------------------------------- samples
    if (command == "set-samples")
    {
        const QString group = arguments["group"].toString();
        if (group.isEmpty())
        {
            error = "set-samples needs a \"group\".";
            return false;
        }
        if (data->count(group.toStdString()) == 0)
        {
            error = "No group named '" + group + "' in this project.";
            return false;
        }

        Elemental_Profile_Set& profiles = data->at(group.toStdString());

        const QStringList only = StringList(arguments, "only");
        const QStringList include = StringList(arguments, "include");
        const QStringList exclude = StringList(arguments, "exclude");

        if (only.isEmpty() && include.isEmpty() && exclude.isEmpty())
        {
            error = "set-samples needs \"only\", \"include\" or \"exclude\".";
            return false;
        }

        for (const QStringList& named : { only, include, exclude })
        {
            for (const QString& sample : named)
            {
                if (profiles.count(sample.toStdString()) == 0)
                {
                    error = "No sample named '" + sample + "' in group '" + group + "'.";
                    return false;
                }
            }
        }

        if (!only.isEmpty())
        {
            for (auto& entry : profiles)
            {
                const QString sample = QString::fromStdString(entry.first);
                const bool wanted = only.contains(sample);
                if (entry.second.IsIncludedInAnalysis() != wanted)
                {
                    entry.second.SetIncludedInAnalysis(wanted);
                    changes << (wanted ? "included sample '" + sample + "'"
                                       : "excluded sample '" + sample + "'");
                }
            }
        }

        for (const QString& sample : include)
        {
            profiles.at(sample.toStdString()).SetIncludedInAnalysis(true);
            changes << "included sample '" + sample + "'";
        }
        for (const QString& sample : exclude)
        {
            profiles.at(sample.toStdString()).SetIncludedInAnalysis(false);
            changes << "excluded sample '" + sample + "'";
        }

        return true;
    }

    // ------------------------------------------------------------------ role
    if (command == "set-element-role")
    {
        const QString element = arguments["element"].toString();
        if (element.isEmpty())
        {
            error = "set-element-role needs an \"element\".";
            return false;
        }

        element_information* information = data->GetElementInformation(element.toStdString());
        if (information == nullptr)
        {
            error = "No constituent named '" + element + "' in this project.";
            return false;
        }

        if (arguments.contains("role"))
        {
            const QString role_name = arguments["role"].toString();
            const QStringList accepted = { "Element", "Isotope", "ParticleSize",
                                           "OM", "DoNotInclude" };
            if (!accepted.contains(role_name))
            {
                error = "'" + role_name + "' is not a role. Use one of: " +
                        accepted.join(", ") + ".";
                return false;
            }
            information->Role = data->Role(role_name);
            changes << "role of '" + element + "' is " + role_name;
        }

        if (arguments.contains("base element"))
        {
            const QString base = arguments["base element"].toString();
            if (!base.isEmpty() &&
                data->GetElementInformation(base.toStdString()) == nullptr)
            {
                error = "No constituent named '" + base + "' to use as the base element.";
                return false;
            }
            information->base_element = base.toStdString();
            changes << "base element of '" + element + "' is '" + base + "'";
        }

        if (arguments.contains("standard ratio"))
        {
            const QJsonValue value = arguments["standard ratio"];
            bool parsed = true;
            const double ratio = value.isString() ? value.toString().toDouble(&parsed)
                                                  : value.toDouble();
            if (!parsed)
            {
                error = "'" + value.toString() + "' is not a number for the standard ratio.";
                return false;
            }
            information->standard_ratio = ratio;
            changes << "standard ratio of '" + element + "' is " + QString::number(ratio);
        }

        data->PopulateConstituentOrders();
        return true;
    }

    error = "Unknown setup command '" + command + "'.";
    return false;
}

} // namespace SetupCommands
