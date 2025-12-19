#ifndef CONDUCTOR_H
#define CONDUCTOR_H

#include <string>
#include <map>
#include <vector>
#include <memory>

#include "sourcesinkdata.h"
#include "GA.h"
#include "MCMC.h"
#include "results.h"

class MainWindow;
class QString;

/**
 * @class Conductor
 * @brief Orchestrates execution of source apportionment analysis operations
 *
 * The Conductor class serves as a command dispatcher and coordinator for various
 * source apportionment algorithms including Genetic Algorithms (GA), Markov Chain
 * Monte Carlo (MCMC), Levenberg-Marquardt optimization, and statistical analyses.
 * It manages the lifecycle of optimization algorithms, maintains analysis results,
 * and provides an interface between the GUI (MainWindow) and the computational
 * backend (SourceSinkData).
 *
 * Key responsibilities:
 * - Execute analysis commands with configurable parameters
 * - Manage algorithm instances (GA, MCMC)
 * - Validate input data for negative concentrations
 * - Collect and package analysis results
 * - Coordinate with progress reporting UI
 *
 * @note The Conductor does not own the SourceSinkData - it operates on externally
 *       managed data passed via SetData()
 */
class Conductor
{
public:
    /**
     * @brief Constructs a Conductor associated with a MainWindow
     * @param mainwindow Pointer to the parent MainWindow (non-owning, must outlive Conductor)
     */
    explicit Conductor(MainWindow* mainwindow);

    /**
     * @brief Executes a specified analysis command with given parameters
     *
     * Supported commands include:
     * - "GA": Genetic Algorithm optimization
     * - "GA (fixed elemental contribution)": GA with fixed source profiles
     * - "GA (disregarding targets)": GA without target constraints
     * - "Levenberg-Marquardt": Nonlinear least squares optimization
     * - "MCMC": Markov Chain Monte Carlo sampling
     * - "Bootstrap": Bootstrap uncertainty analysis
     * - "ANOVA": Analysis of variance for source discrimination
     * - "Error_Analysis": Error propagation analysis
     * - "Source_Verify": Source verification analysis
     * - "AutoSelect": Automated element selection
     *
     * @param command Command name identifying the analysis type
     * @param arguments Map of parameter names to values configuring the analysis
     * @return true if execution completed successfully, false if validation failed or errors occurred
     *
     * @note Clears previous results at the start of each execution
     * @note Validates data for negative element concentrations before proceeding
     */
    bool Execute(const std::string& command, std::map<std::string, std::string> arguments);

    /**
     * @brief Returns pointer to the current SourceSinkData
     * @return Non-owning pointer to the data object, or nullptr if not set
     */
    SourceSinkData* Data() { return data; }

    /**
     * @brief Sets the SourceSinkData for analysis
     * @param _data Pointer to SourceSinkData (non-owning, must remain valid during operations)
     */
    void SetData(SourceSinkData* _data) { data = _data; }

    /**
     * @brief Creates and returns a heap-allocated copy of current results
     *
     * Returns a new Results object allocated on the heap containing a copy of
     * the most recent analysis results. The caller assumes ownership and is
     * responsible for deletion.
     *
     * @return Heap-allocated Results pointer (caller must delete)
     *
     * @note This allocates memory - caller must ensure proper cleanup
     * @warning Results are cleared at the start of each Execute() call, so this
     *          should be called before the next analysis
     */
    Results* GetResults() { return new Results(results); }

    /**
     * @brief Sets the working directory for output files
     * @param wf Working folder path as QString
     */
    void SetWorkingFolder(const QString& wf) { workingfolder = wf; }

    /**
     * @brief Returns the current working directory path
     * @return Working folder path as QString
     */
    QString WorkingFolder() const { return workingfolder; }

    /**
     * @brief Validates that all element concentrations are non-negative
     *
     * Checks the specified SourceSinkData for negative element concentrations,
     * which are physically invalid for most source apportionment analyses
     * (except isotope delta values). Displays a warning dialog if negative
     * values are found.
     *
     * @param data Pointer to SourceSinkData to check (uses this->data if nullptr)
     * @return true if all concentrations are valid (non-negative), false otherwise
     *
     * @note Presents warning dialog to user via MainWindow if validation fails
     */
    bool CheckNegativeElements(SourceSinkData* data = nullptr);

    /**
     * @brief Validates element concentrations across multiple samples
     *
     * Checks for negative concentrations in a collection of samples, where each
     * map entry represents a sample and its list of problematic elements.
     *
     * @param negative_elements Map from sample names to lists of elements with negative values
     * @return true if no negative values found, false if any exist
     *
     * @note Presents consolidated warning dialog showing all problematic samples
     */
    bool CheckNegativeElements(std::map<std::string, std::vector<std::string>> negative_elements);

private:
    SourceSinkData* data;                          ///< Non-owning pointer to analysis data
    std::unique_ptr<CGA<SourceSinkData>> GA;       ///< Genetic Algorithm optimizer (owned)
    std::unique_ptr<CMCMC<SourceSinkData>> MCMC;   ///< MCMC sampler (owned)
    Results results;                                ///< Current analysis results (cleared each Execute)
    QString workingfolder;                          ///< Output directory path
    MainWindow* mainwindow;                         ///< Non-owning pointer to parent window
};

#endif // CONDUCTOR_H