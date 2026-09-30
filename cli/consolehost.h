#ifndef CONSOLEHOST_H
#define CONSOLEHOST_H

#include "analysishost.h"
#include "progressreporter.h"

#include <vector>
#include <string>

/**
 * @class ConsoleProgressReporter
 * @brief ProgressReporter that writes progress to the standard error stream
 *
 * Progress is written as a percentage on a single rewritten line, and only
 * when the reported figure has advanced by at least a whole percent, so that a
 * run producing thousands of updates does not produce thousands of lines. The
 * chart operations have no console equivalent and are discarded.
 *
 * Everything is written to standard error, leaving standard output free for
 * the run report.
 */
class ConsoleProgressReporter : public ProgressReporter
{
public:
    /**
     * @brief Constructs a reporter
     * @param quiet When true, nothing is written at all
     */
    explicit ConsoleProgressReporter(bool quiet);

    void Start() override;
    void Finish() override;

    void SetProgress(const double& prog) override;
    void SetProgress2(const double& prog) override;
    void SetLabel(const QString& label) override;

    // No console equivalent; progress charts are discarded.
    void AppendPoint(const double&, const double&, int) override {}
    void SetYRange(const double&, const double&, int) override {}
    void SetXRange(const double&, const double&, int) override {}
    void SetTitle(const QString&, int) override {}
    void SetXAxisTitle(const QString&, int) override {}
    void SetYAxisTitle(const QString&, int) override {}
    void ClearGraph(int) override {}
    void SetChartTitle(int, const QString&) override {}

private:
    void Write(const std::string& text);

    bool quiet;                 ///< Suppresses all output when set
    int last_reported_percent;  ///< Last percentage written, to avoid repeating
    std::string label;          ///< Most recent status label
    bool wrote_anything;        ///< Whether a line needs terminating on Finish()
};

/**
 * @class ConsoleHost
 * @brief AnalysisHost for a run with no graphical interface
 *
 * Warnings are written to standard error and also retained, so that the run
 * report can record what an analysis complained about rather than losing it to
 * the terminal. Progress goes to ConsoleProgressReporter.
 *
 * @see GuiHost for the graphical counterpart
 */
class ConsoleHost : public AnalysisHost
{
public:
    /**
     * @brief Constructs a host
     * @param quiet When true, suppresses progress output; warnings are still
     *        written, since they explain why an analysis did not run
     */
    explicit ConsoleHost(bool quiet = false);
    ~ConsoleHost() override;

    void ShowWarning(const std::string& message) override;

    ProgressReporter* CreateProgressReporter(int number_of_panels = 1,
                                             bool extra_label_and_progressbar = false) override;

    void DisposeProgressReporter(ProgressReporter* reporter) override;

    /// @brief Warnings reported since the last call to ClearWarnings()
    const std::vector<std::string>& Warnings() const { return warnings; }

    /// @brief Discards the retained warnings, called between steps
    void ClearWarnings() { warnings.clear(); }

private:
    bool quiet;                                    ///< Suppresses progress output
    std::vector<std::string> warnings;             ///< Warnings retained for the report
    std::vector<ConsoleProgressReporter*> reporters; ///< Outstanding reporters, owned
};

#endif // CONSOLEHOST_H
