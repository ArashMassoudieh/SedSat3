#ifndef PROGRESSREPORTER_H
#define PROGRESSREPORTER_H

#include <QString>

/**
 * @class ProgressReporter
 * @brief Abstract sink for progress information produced by a running analysis
 *
 * The long-running algorithms (genetic algorithm, MCMC, distribution fitting,
 * the batch analyses) report their progress as they go. In the graphical
 * application that progress is drawn in a ProgressWindow; in the command line
 * application it is written to the console or discarded. Neither the algorithms
 * nor the Conductor should care which, so both report through this interface.
 *
 * The method set mirrors the ProgressWindow API that the algorithms were
 * already written against, so that ProgressWindow implements this interface
 * without changing the body of any of its methods.
 *
 * Only QString is used here, which belongs to QtCore. This header deliberately
 * pulls in nothing from QtWidgets, so that code including it stays usable in a
 * program that never creates a window.
 *
 * @note Implementations must tolerate being called from an analysis that runs
 *       to completion without ever calling Start() or Finish()
 * @see ProgressWindow for the graphical implementation
 * @see AnalysisHost for how reporters are created and disposed of
 */
class ProgressReporter
{
public:
    virtual ~ProgressReporter() = default;

    /**
     * @brief Makes the reporter visible or otherwise begins reporting
     *
     * Named Start() rather than show() because QWidget::show() is not virtual,
     * so a widget-derived implementation cannot override a show() declared
     * here.
     */
    virtual void Start() {}

    /**
     * @brief Ends reporting
     *
     * Named Finish() for the same reason Start() is not called show().
     * Disposing of the reporter is the responsibility of the AnalysisHost that
     * created it, not of this call.
     */
    virtual void Finish() {}

    /**
     * @brief Adds a point to one of the progress charts
     * @param x Horizontal coordinate, typically an iteration or generation number
     * @param y Vertical coordinate, typically a fitness or likelihood value
     * @param chart Index of the chart to append to
     */
    virtual void AppendPoint(const double& x, const double& y, int chart = 0) = 0;

    /**
     * @brief Sets the primary progress fraction
     * @param prog Progress in the range [0,1]
     */
    virtual void SetProgress(const double& prog) = 0;

    /**
     * @brief Sets the secondary progress fraction, where one is displayed
     * @param prog Progress in the range [0,1]
     */
    virtual void SetProgress2(const double& prog) = 0;

    /// @brief Sets the free-text status label
    virtual void SetLabel(const QString& label) = 0;

    /// @brief Sets the vertical range of a chart
    virtual void SetYRange(const double& y0, const double& y1, int chart = 0) = 0;

    /// @brief Sets the horizontal range of a chart
    virtual void SetXRange(const double& x0, const double& x1, int chart = 0) = 0;

    /// @brief Sets the title of a chart
    virtual void SetTitle(const QString& title, int chart = 0) = 0;

    /// @brief Sets the horizontal axis title of a chart
    virtual void SetXAxisTitle(const QString& title, int chart = 0) = 0;

    /// @brief Sets the vertical axis title of a chart
    virtual void SetYAxisTitle(const QString& title, int chart = 0) = 0;

    /// @brief Discards the points accumulated in a chart
    virtual void ClearGraph(int chart = 0) = 0;

    /// @brief Sets the title of chart @p i
    virtual void SetChartTitle(int i, const QString& title) = 0;
};

/**
 * @class NullProgressReporter
 * @brief ProgressReporter that discards everything reported to it
 *
 * Used where an analysis is run without any progress display, so that the
 * analysis code can report unconditionally instead of testing the reporter
 * against nullptr at every call site.
 */
class NullProgressReporter : public ProgressReporter
{
public:
    void AppendPoint(const double&, const double&, int) override {}
    void SetProgress(const double&) override {}
    void SetProgress2(const double&) override {}
    void SetLabel(const QString&) override {}
    void SetYRange(const double&, const double&, int) override {}
    void SetXRange(const double&, const double&, int) override {}
    void SetTitle(const QString&, int) override {}
    void SetXAxisTitle(const QString&, int) override {}
    void SetYAxisTitle(const QString&, int) override {}
    void ClearGraph(int) override {}
    void SetChartTitle(int, const QString&) override {}
};

#endif // PROGRESSREPORTER_H
