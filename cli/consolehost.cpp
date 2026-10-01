#include "consolehost.h"

#include <QString>

#include <algorithm>
#include <iostream>

ConsoleProgressReporter::ConsoleProgressReporter(bool _quiet)
    : quiet(_quiet),
      last_reported_percent(-1),
      wrote_anything(false)
{
}

void ConsoleProgressReporter::Write(const std::string& text)
{
    if (quiet)
        return;

    std::cerr << text << std::flush;
    wrote_anything = true;
}

void ConsoleProgressReporter::Start()
{
    last_reported_percent = -1;
}

void ConsoleProgressReporter::Finish()
{
    if (wrote_anything)
    {
        std::cerr << std::endl;
        wrote_anything = false;
    }
    last_reported_percent = -1;
}

void ConsoleProgressReporter::SetProgress(const double& prog)
{
    // Analyses report a fraction; clamp it because a few compute it from a
    // ratio that can overshoot slightly.
    const double clamped = std::max(0.0, std::min(1.0, prog));
    const int percent = static_cast<int>(clamped * 100.0);

    if (percent == last_reported_percent)
        return;

    last_reported_percent = percent;

    std::string line = "\r  " + std::to_string(percent) + "%";
    if (!label.empty())
        line += "  " + label;
    line += "    ";

    Write(line);
}

void ConsoleProgressReporter::SetProgress2(const double& prog)
{
    // The secondary bar tracks the outer loop of a batch analysis. Reporting
    // both on one line is noise, so only the primary figure is shown.
    (void)prog;
}

void ConsoleProgressReporter::SetLabel(const QString& _label)
{
    label = _label.toStdString();
}

ConsoleHost::ConsoleHost(bool _quiet)
    : quiet(_quiet)
{
}

ConsoleHost::~ConsoleHost()
{
    for (ConsoleProgressReporter* reporter : reporters)
        delete reporter;
    reporters.clear();
}

void ConsoleHost::ShowWarning(const std::string& message)
{
    // Retained as well as written, so the run report records why an analysis
    // declined to run instead of leaving it only on the terminal.
    warnings.push_back(message);
    std::cerr << "  warning: " << message << std::endl;
}

ProgressReporter* ConsoleHost::CreateProgressReporter(int number_of_panels,
                                                      bool extra_label_and_progressbar)
{
    (void)number_of_panels;
    (void)extra_label_and_progressbar;

    ConsoleProgressReporter* reporter = new ConsoleProgressReporter(quiet);
    reporters.push_back(reporter);
    return reporter;
}

void ConsoleHost::DisposeProgressReporter(ProgressReporter* reporter)
{
    if (reporter == nullptr)
        return;

    for (size_t i = 0; i < reporters.size(); i++)
    {
        if (reporters[i] == reporter)
        {
            reporters[i]->Finish();
            delete reporters[i];
            reporters.erase(reporters.begin() + i);
            return;
        }
    }
}
