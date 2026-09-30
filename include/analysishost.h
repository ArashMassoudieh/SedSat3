#ifndef ANALYSISHOST_H
#define ANALYSISHOST_H

#include <string>

class ProgressReporter;

/**
 * @class AnalysisHost
 * @brief Abstract environment that an analysis runs inside
 *
 * The Conductor needs two things from the program it is running in: somewhere
 * to report a problem that stops an analysis, and a way to obtain a progress
 * reporter for a long-running one. In the graphical application those are a
 * message box and a ProgressWindow; in the command line application they are
 * console output and a text reporter. The Conductor is written against this
 * interface so that it contains no reference to either.
 *
 * The host owns every reporter it hands out. A caller that obtains one with
 * CreateProgressReporter() must return it with DisposeProgressReporter() once
 * the analysis is finished, and must not delete it directly.
 *
 * @note This header pulls in nothing from Qt, so it is usable from code that
 *       never creates a window
 * @see ProgressReporter
 */
class AnalysisHost
{
public:
    virtual ~AnalysisHost() = default;

    /**
     * @brief Reports a condition that prevented an analysis from running
     *
     * Called when input is rejected before or during an analysis, for example
     * zero or negative concentrations, an unusable isotope configuration, or a
     * correction that has not been performed yet.
     *
     * @param message Text describing the problem, possibly several lines
     */
    virtual void ShowWarning(const std::string& message) = 0;

    /**
     * @brief Creates a progress reporter for a long-running analysis
     *
     * The returned reporter is owned by the host and remains valid until it is
     * passed to DisposeProgressReporter().
     *
     * @param number_of_panels Number of progress charts the analysis will use
     * @param extra_label_and_progressbar Whether a second label and progress
     *        bar are needed, for analyses that report nested progress
     * @return Reporter to pass to the analysis, never nullptr
     */
    virtual ProgressReporter* CreateProgressReporter(int number_of_panels = 1,
                                                     bool extra_label_and_progressbar = false) = 0;

    /**
     * @brief Releases a reporter previously obtained from this host
     *
     * Passing nullptr is permitted and does nothing.
     *
     * @param reporter Reporter to release; must not be used afterwards
     */
    virtual void DisposeProgressReporter(ProgressReporter* reporter) = 0;
};

#endif // ANALYSISHOST_H
