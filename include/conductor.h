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

    /**
     * @brief Executes Genetic Algorithm optimization
     *
     * Performs standard GA optimization with configurable source profiles,
     * organic matter and size corrections, and comprehensive result collection
     * including contributions, observed vs modeled profiles, and statistical
     * parameters (means, mu, sigma) for both calculated and estimated values.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Sample": Target sample name
     *                  - "Apply size and organic matter correction": "true"/"false"
     *                  - Plus other GA-specific parameters passed to SetProperties
     * @return true if execution completed successfully, false if validation failed
     */
    bool ExecuteGA(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes Genetic Algorithm with fixed elemental profiles
     *
     * Performs GA optimization where source elemental profiles are held constant
     * and only source contributions are estimated. This mode is useful when source
     * compositions are well-characterized and only mixing proportions need determination.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Sample": Target sample name
     *                  - "Apply size and organic matter correction": "true"/"false"
     *                  - Plus other GA-specific parameters
     * @return true if execution completed successfully, false if validation failed
     */
    bool ExecuteGA_FixedProfile(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes Genetic Algorithm disregarding target constraints
     *
     * Performs GA optimization to estimate source elemental profiles based solely
     * on source data, without constraining the solution to match target sample
     * compositions. This mode is useful for characterizing source profiles when
     * target data is unavailable or unreliable.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Sample": Target sample name (used for naming only)
     *                  - Plus other GA-specific parameters
     * @return true if execution completed successfully, false if validation failed
     */
    bool ExecuteGA_NoTargets(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes Levenberg-Marquardt nonlinear optimization
     *
     * Performs Levenberg-Marquardt least squares optimization to estimate source
     * contributions. This deterministic algorithm is faster than GA/MCMC and works
     * well when the problem is well-conditioned. Supports both linear and softmax
     * transformations for contribution constraints.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Sample": Target sample name
     *                  - "Apply size and organic matter correction": "true"/"false"
     *                  - "Softmax transformation": "true"/"false"
     * @return true if execution completed successfully, false if validation failed
     */
    bool ExecuteLevenbergMarquardt(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes Levenberg-Marquardt optimization across all target samples
     *
     * Performs batch Levenberg-Marquardt optimization for all target samples,
     * producing a comprehensive contribution matrix showing source apportionment
     * results across multiple samples. Useful for analyzing spatial or temporal
     * patterns in source contributions.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Apply size and organic matter correction": "true"/"false"
     *                  - "Softmax transformation": "true"/"false"
     * @return true if execution completed successfully, false if validation failed
     */
    bool ExecuteLevenbergMarquardtBatch(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes organic matter and particle size correction
     *
     * Applies multiple linear regression corrections to adjust elemental concentrations
     * for variations in organic matter content and particle size. Returns corrected
     * source profiles for the specified target sample, which can then be used in
     * subsequent source apportionment analyses.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Sample": Target sample name for correction basis
     *                  - "Use only selected samples": "true"/"false"
     *                  - "Use only selected elements": "true"/"false"
     * @return true if execution completed successfully
     */
    bool ExecuteOMSizeCorrect(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes multiple linear regression versus organic matter and particle size
     *
     * Performs multiple linear regression analysis to establish relationships between
     * elemental concentrations and organic matter content/particle size. The resulting
     * regression models are stored and can be used to correct concentrations in
     * subsequent analyses. Supports both linear and power-law regression forms.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Organic Matter constituent": OM variable name (can be empty)
     *                  - "Particle Size constituent": Size variable name (can be empty)
     *                  - "Equation": "Linear" or "Power"
     *                  - "P-value threshold": Statistical significance threshold
     *                  - "Use only selected samples": "true"/"false"
     * @return true if execution completed successfully, false if no constituents selected
     */
    bool ExecuteMLR(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Calculates covariance matrix for a source or target group
     *
     * Computes the covariance matrix showing relationships between elemental
     * concentrations within a specified source or target group. Useful for
     * understanding correlations and dependencies between elements in the data.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Source/Target group": Name of the group to analyze
     * @return true if execution completed successfully
     */
    bool ExecuteCovarianceMatrix(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Calculates correlation matrix for a source or target group
     *
     * Computes the correlation matrix showing normalized relationships between
     * elemental concentrations within a specified source or target group. Supports
     * optional organic matter/size corrections and Box-Cox transformations. Values
     * outside the specified threshold are highlighted.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Source/Target group": Name of the group to analyze
     *                  - "Threshold": Correlation threshold for highlighting
     *                  - "Use only selected samples": "true"/"false"
     *                  - "Use only selected elements": "true"/"false"
     *                  - "OM and Size Correct based on target sample": Sample name or empty
     * @return true if execution completed successfully, false if OM/Size correction unavailable
     */
    bool ExecuteCorrelationMatrix(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes Discriminant Function Analysis between two groups
     *
     * Performs two-group discriminant function analysis to find the linear combination
     * of elements that best separates two source or target groups. Computes eigenvalues,
     * eigenvectors, projected profiles, and statistical significance (chi-squared and
     * F-test p-values). Supports optional Box-Cox transformation and OM/size corrections.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Source/Target group I": First group name
     *                  - "Source/Target group II": Second group name
     *                  - "Use only selected samples": "true"/"false"
     *                  - "Use only selected elements": "true"/"false"
     *                  - "OM and Size Correct based on target sample": Sample name or empty
     *                  - "Box-cox transformation": "true"/"false"
     * @return true if execution completed successfully, false if groups are identical or matrix is singular
     */
    bool ExecuteDFA(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes Discriminant Function Analysis for one group versus all others
     *
     * Performs discriminant function analysis to separate one specified source group
     * from all other groups combined. Useful for identifying which elements best
     * distinguish a particular source from the background of all other sources.
     * Supports optional Box-Cox transformation and OM/size corrections.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Source group": Group name to discriminate from others
     *                  - "Use only selected samples": "true"/"false"
     *                  - "Use only selected elements": "true"/"false"
     *                  - "OM and Size Correct based on target sample": Sample name or empty
     *                  - "Box-cox transformation": "true"/"false"
     * @return true if execution completed successfully, false if matrix is singular
     */
    bool ExecuteDFAOnevsRest(const std::map<std::string, std::string>& arguments);
    /**
     * @brief Executes multi-way Discriminant Function Analysis across all groups
     *
     * Performs multi-way discriminant function analysis to find linear combinations
     * of elements that optimally separate all source groups simultaneously. Computes
     * multiple discriminant functions (eigenvalues/eigenvectors) and projects all
     * groups onto these discriminant axes. Supports optional Box-Cox transformation
     * and OM/size corrections.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Use only selected samples": "true"/"false"
     *                  - "Use only selected elements": "true"/"false"
     *                  - "OM and Size Correct based on target sample": Sample name or empty
     *                  - "Box-cox transformation": "true"/"false"
     * @return true if execution completed successfully, false if matrix is singular or negative elements found
     */
    bool ExecuteDFAM(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes Stepwise Discriminant Function Analysis between two groups
     *
     * Performs stepwise discriminant function analysis to identify the subset of
     * elements that provides optimal discrimination between two source groups.
     * Elements are added sequentially based on their contribution to group separation,
     * evaluated using Wilks' Lambda, chi-squared, and F-test statistics. Supports
     * optional Box-Cox transformation and OM/size corrections.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Source/Target group I": First group name
     *                  - "Source/Target group II": Second group name
     *                  - "Use only selected samples": "true"/"false"
     *                  - "Use only selected elements": "true"/"false"
     *                  - "OM and Size Correct based on target sample": Sample name or empty
     *                  - "Box-cox transformation": "true"/"false"
     * @return true if execution completed successfully, false if groups identical, matrix singular, or negative elements found
     */
    bool ExecuteSDFA(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes multi-way Stepwise Discriminant Function Analysis across all groups
     *
     * Performs stepwise discriminant function analysis to identify the optimal subset
     * of elements for discriminating among all source groups simultaneously. Elements
     * are sequentially selected based on their contribution to overall group separation
     * using Wilks' Lambda, chi-squared, and F-test statistics. Optionally modifies
     * the included elements based on results. Supports Box-Cox transformation and
     * OM/size corrections.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Use only selected samples": "true"/"false"
     *                  - "Use only selected elements": "true"/"false"
     *                  - "OM and Size Correct based on target sample": Sample name or empty
     *                  - "Box-cox transformation": "true"/"false"
     *                  - "Modify the included elements based on the results": "true"/"false"
     * @return true if execution completed successfully, false if matrix singular or negative elements found
     */
    bool ExecuteSDFAM(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes Stepwise Discriminant Function Analysis for one group versus all others
     *
     * Performs stepwise discriminant function analysis to identify the optimal subset
     * of elements for discriminating one specified source group from all other groups
     * combined. Elements are sequentially selected based on Wilks' Lambda, chi-squared,
     * and F-test statistics. Supports optional Box-Cox transformation and OM/size
     * corrections.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Source group": Group name to discriminate from others
     *                  - "Use only selected samples": "true"/"false"
     *                  - "Use only selected elements": "true"/"false"
     *                  - "OM and Size Correct based on target sample": Sample name or empty
     *                  - "Box-cox transformation": "true"/"false"
     * @return true if execution completed successfully, false if matrix singular or negative elements found
     */
    bool ExecuteSDFAOnevsRest(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes Kolmogorov-Smirnov goodness-of-fit test for a source/target group
     *
     * Performs Kolmogorov-Smirnov test to assess how well elemental concentration
     * distributions within a source or target group conform to a specified theoretical
     * distribution (normal or lognormal). Returns K-S statistics for each element,
     * where larger values indicate poorer fit to the assumed distribution.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Source/Target group": Name of the group to analyze
     *                  - "Distribution": "Normal" or "Lognormal"
     * @return true if execution completed successfully
     */
    bool ExecuteKolmogorovSmirnov(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes Kolmogorov-Smirnov test for a single constituent in a group
     *
     * Performs detailed Kolmogorov-Smirnov goodness-of-fit test for one specific
     * element within a source or target group. Returns the cumulative distribution
     * function (CDF) comparison between observed data and the theoretical distribution
     * (normal or lognormal), allowing visual assessment of distribution fit.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Source/Target group": Name of the group to analyze
     *                  - "Constituent": Specific element name to test
     *                  - "Distribution": "Normal" or "Lognormal"
     * @return true if execution completed successfully
     */
    bool ExecuteKolmogorovSmirnovIndividual(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes Bayesian Chemical Mass Balance using MCMC sampling
     *
     * Performs Bayesian inference for source apportionment using Markov Chain Monte
     * Carlo (MCMC) sampling. This probabilistic approach estimates posterior distributions
     * of source contributions accounting for uncertainties in both source profiles and
     * receptor measurements. Generates comprehensive results including contribution
     * distributions, parameter traces, and convergence diagnostics. Supports optional
     * organic matter/size corrections and softmax transformations.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Sample": Target sample name
     *                  - "Apply size and organic matter correction": "true"/"false"
     *                  - Plus MCMC-specific parameters (chain count, samples, burnin, etc.)
     * @return true if execution completed successfully, false if OM/Size correction unavailable or errors occur
     */
    bool ExecuteCMBBayesian(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes Bayesian Chemical Mass Balance across all target samples
     *
     * Performs batch MCMC-based Bayesian source apportionment for all target samples,
     * producing a comprehensive contribution matrix showing probabilistic source
     * contributions with uncertainty ranges across multiple samples. This is the batch
     * version of CMB Bayesian, enabling efficient spatial or temporal analysis of
     * source contributions. Supports optional organic matter/size corrections.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Apply size and organic matter correction": "true"/"false"
     *                  - Plus MCMC-specific parameters (chain count, samples, burnin, etc.)
     * @return true if execution completed successfully, false if OM/Size correction unavailable
     */
    bool ExecuteCMBBayesianBatch(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes MCMC test with a simple analytical model
     *
     * Performs MCMC sampling on a simple test model (TestMCMC) with known analytical
     * properties to validate the MCMC implementation. Useful for debugging convergence
     * issues, tuning sampler parameters, and verifying posterior distributions against
     * expected theoretical results. Generates both parameter trace samples and posterior
     * distributions.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Number of samples": Total MCMC samples per chain
     *                  - "Number of chains": Number of independent chains
     *                  - "Samples to be discarded (burnout)": Burnin period
     *                  - "samples_file_name": Output file name for samples
     * @return true if execution completed successfully
     */
    bool ExecuteTestCMBBayesian(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes distribution fitting analysis for a constituent
     *
     * Fits normal and lognormal distributions to observed elemental concentrations
     * for a specific constituent within a source or target group. Generates both
     * probability density functions (PDF) and cumulative distribution functions (CDF)
     * comparing observed data with fitted theoretical distributions. Useful for
     * assessing distributional assumptions before source apportionment. Supports
     * optional Box-Cox transformation and OM/size corrections.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Source/Target group": Name of the group to analyze
     *                  - "Constituent": Specific element name to fit
     *                  - "Use only selected samples": "true"/"false"
     *                  - "OM and Size Correct based on target sample": Sample name or empty
     *                  - "Box-cox transformation": "true"/"false"
     * @return true if execution completed successfully, false if OM/Size correction unavailable or negative elements found
     */
    bool ExecuteDistributionFitting(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes bracketing analysis for a single target sample
     *
     * Performs bracketing test to determine if target sample concentrations fall
     * within the range defined by source group concentrations. For each element,
     * the test checks whether the target value is bracketed (bounded) by the minimum
     * and maximum source values. Results are boolean indicators showing which elements
     * pass the bracketing criterion. Supports optional OM/size corrections.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Sample": Target sample name to test
     *                  - "Use only selected samples": "true"/"false"
     *                  - "Use only selected elements": "true"/"false"
     *                  - "Correct based on size and organic matter": "true"/"false"
     * @return true if execution completed successfully, false if negative elements found
     */
    bool ExecuteBracketingAnalysis(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes bracketing analysis across all target samples
     *
     * Performs batch bracketing test to determine if target sample concentrations
     * fall within the range defined by source group concentrations for all samples.
     * Returns a matrix showing boolean bracketing results for each element in each
     * target sample. This batch analysis enables quick assessment of which samples
     * and elements satisfy the bracketing criterion. Supports optional OM/size
     * corrections.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Use only selected samples": "true"/"false"
     *                  - "Use only selected elements": "true"/"false"
     *                  - "Correct based on size and organic matter": "true"/"false"
     * @return true if execution completed successfully, false if OM/Size correction unavailable or negative elements found
     */
    bool ExecuteBracketingAnalysisBatch(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Calculates optimal Box-Cox transformation parameters
     *
     * Computes the optimal Box-Cox lambda parameters for variance-stabilizing
     * power transformations of elemental concentrations within a source or target
     * group. Box-Cox transformation can normalize skewed distributions and stabilize
     * variance, improving the performance of subsequent statistical analyses.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Source/Target group": Name of the group to analyze
     * @return true if execution completed successfully
     */
    bool ExecuteBoxCox(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes outlier detection analysis for a source/target group
     *
     * Performs Box-Cox based outlier detection to identify samples with unusual
     * elemental concentrations within a source or target group. After Box-Cox
     * transformation to normality, calculates standardized residuals and flags
     * values exceeding the specified threshold (typically ±2 or ±3 standard
     * deviations). Supports sample and element filtering.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Source/Target group": Name of the group to analyze
     *                  - "Threshold": Standard deviation threshold for outlier detection
     *                  - "Use only selected samples": "true"/"false"
     *                  - "Use only selected elements": "true"/"false"
     * @return true if execution completed successfully, false if negative elements found
     */
    bool ExecuteOutlierAnalysis(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Calculates two-way element discriminant power between two groups
     *
     * Evaluates the ability of each element to discriminate between two specific
     * source or target groups using multiple metrics: difference-to-standard-deviation
     * ratio, discrimination fraction (percentage of samples correctly separated),
     * and statistical significance (t-test p-values). Elements with low p-values
     * and high discrimination power are good candidates for source apportionment.
     * Supports Box-Cox transformation and OM/size corrections.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Source/Target group I": First group name
     *                  - "Source/Target group II": Second group name
     *                  - "Use only selected samples": "true"/"false"
     *                  - "OM and Size Correct based on target sample": Sample name or empty
     *                  - "Box-cox transformation": "true"/"false"
     *                  - "P-value threshold": Significance threshold for highlighting
     * @return true if execution completed successfully, false if OM/Size correction unavailable or negative elements found
     */
    bool ExecuteEDP(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Calculates multi-way element discriminant power across all groups
     *
     * Evaluates the ability of each element to discriminate among all source groups
     * simultaneously using pairwise comparisons. Computes three metrics for each
     * element-pair combination: difference-to-standard-deviation ratio, discrimination
     * fraction, and t-test p-values. Results show which elements provide optimal
     * discrimination across the entire source dataset. Optionally includes target
     * samples. Supports Box-Cox transformation and OM/size corrections.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Include target samples": "true"/"false"
     *                  - "Use only selected samples": "true"/"false"
     *                  - "OM and Size Correct based on target sample": Sample name or empty
     *                  - "Box-cox transformation": "true"/"false"
     *                  - "P-value threshold": Significance threshold for highlighting
     * @return true if execution completed successfully, false if OM/Size correction unavailable or negative elements found
     */
    bool ExecuteEDPM(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes Analysis of Variance (ANOVA) for element discrimination
     *
     * Performs one-way ANOVA to test whether elemental concentrations differ
     * significantly across source groups. Returns p-values for each element
     * indicating the statistical significance of between-group differences.
     * Elements with low p-values (below threshold) show significant variation
     * across sources and are good candidates for source apportionment. Optionally
     * modifies included elements based on results. Supports log transformation
     * and Box-Cox transformation.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Log Transformation": "true"/"false"
     *                  - "Use only selected samples": "true"/"false"
     *                  - "Use only selected elements": "true"/"false"
     *                  - "OM and Size Correct based on target sample": Sample name or empty
     *                  - "Box-cox transformation": "true"/"false"
     *                  - "P-value threshold": Significance threshold
     *                  - "Modify the included elements based on the results": "true"/"false"
     * @return true if execution completed successfully, false if OM/Size correction unavailable or negative elements found
     */
    bool ExecuteANOVA(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes bootstrap error analysis for uncertainty quantification
     *
     * Performs bootstrap resampling to quantify uncertainty in source contribution
     * estimates. Randomly removes a specified percentage of elements in each bootstrap
     * iteration and re-solves the source apportionment problem, generating distributions
     * of contribution estimates. Produces confidence intervals and uncertainty metrics
     * for source contributions. Supports both linear and softmax transformations and
     * optional OM/size corrections.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Sample": Target sample name
     *                  - "Apply size and organic matter correction": "true"/"false"
     *                  - "Softmax transformation": "true"/"false"
     *                  - "Pecentage eliminated": Percentage of elements to remove per iteration
     *                  - "Number of realizations": Number of bootstrap iterations
     * @return true if execution completed successfully, false if OM/Size correction unavailable or negative elements found
     */
    bool ExecuteErrorAnalysis(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes source verification analysis
     *
     * Performs leave-one-out cross-validation where each sample from a specified
     * source group is treated as an unknown target and contributions are estimated
     * using the remaining sources. If source apportionment is working correctly,
     * the "unknown" sample should show high contribution from its true source group.
     * Results show contribution matrices for all verification samples, enabling
     * assessment of source profile representativeness and model reliability. Supports
     * softmax transformation and optional OM/size corrections.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "Source Group": Source group name to verify
     *                  - "Apply size and organic matter correction": "true"/"false"
     *                  - "Softmax transformation": "true"/"false"
     * @return true if execution completed successfully, false if OM/Size correction unavailable or negative elements found
     */
    bool ExecuteSourceVerify(const std::map<std::string, std::string>& arguments);

    /**
     * @brief Executes automatic element selection based on discriminant power
     *
     * Automatically selects the optimal subset of elements for source apportionment
     * by analyzing multi-way discriminant p-values for all source group pairs.
     * Elements are ranked by their ability to discriminate between sources, and
     * the top N elements from each pairwise comparison are aggregated to form the
     * final selection. Optionally modifies the included elements in the dataset.
     * Supports isotope inclusion, Box-Cox transformation, and OM/size corrections.
     *
     * @param arguments Map of configuration parameters including:
     *                  - "OM and Size Correct based on target sample": Sample name or empty
     *                  - "Include Isotopes": "true"/"false"
     *                  - "Number of elements from each pair": Top N elements per pair
     *                  - "Modify the included elements based on the results": "true"/"false"
     * @return true if execution completed successfully, false if OM/Size correction unavailable or negative elements found
     */
    bool ExecuteAutoSelect(const std::map<std::string, std::string>& arguments);
};

#endif // CONDUCTOR_H