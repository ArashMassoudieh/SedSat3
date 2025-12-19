#ifndef SOURCESINKDATA_H
#define SOURCESINKDATA_H

#include <elemental_profile_set.h>
#include "vector"
#include "parameter.h"
#include "observation.h"
#include "contribution.h"
#include "results.h"
#include "ProgressWindow.h"
#include "fstream"
#include "qjsonobject.h"
#include "cmbvector.h"
#include "cmbvectorset.h"
#include "cmbvectorsetset.h"
#include "MCMC.h"

enum class transformation { linear, softmax };

struct profiles_data
{
    vector<string> element_names;
    vector<string> sample_names;
    vector<vector<double>> values;
};

struct element_data
{
    string group_name;
    vector<string> sample_names;
    vector<double> values;
};

struct element_data_groups
{
    vector<string> group_names;
    vector<string> sample_names;
    vector<vector<double>> values;
};

struct DFA_result
{
    CMBVectorSet eigen_vectors;
    CMBVectorSet projected;
    CMBVector p_values;
    CMBVector wilkslambda;
    CMBVector F_test_P_value;
    CMBVectorSetSet multi_projected;
};

struct ANOVA_info
{
    double F = 0;
    double SST = 0;
    double SSB = 0;
    double SSW = 0;
    double MSB = 0;
    double MSW = 0;
    double p_value = 0;
};

enum class estimation_mode { only_contributions, elemental_profile_and_contribution, source_elemental_profiles_based_on_source_data };

enum class negative_reporting { list, complete };

class SourceSinkData : public map<string, Elemental_Profile_Set>
{
public:
    // ========== Construction and Assignment ==========

    /**
     * @brief Default constructor
     *
     * Initializes an empty SourceSinkData object with default options.
     * Sets outlier deviation threshold to 3.0 by default.
     */
    SourceSinkData();

    /**
     * @brief Copy constructor
     *
     * Creates a deep copy of another SourceSinkData object, including all
     * elemental profile sets, element information, distributions, parameters,
     * observations, and analysis settings.
     *
     * @param other The SourceSinkData object to copy from
     */
    SourceSinkData(const SourceSinkData& other);

    /**
     * @brief Assignment operator
     *
     * Assigns all data from another SourceSinkData object to this one.
     * Performs deep copy of all member variables.
     *
     * @param other The SourceSinkData object to assign from
     * @return Reference to this object
     */
    SourceSinkData& operator=(const SourceSinkData& other);

    // ========== Data Loading and Management ==========

    /**
     * @brief Clear all data from the object
     *
     * Resets the SourceSinkData object to its initial empty state by:
     * - Clearing all profile sets (sources and targets)
     * - Clearing element information and distributions
     * - Resetting all counters to zero
     * - Clearing parameters and observations
     * - Clearing all ordering vectors
     * - Resetting file paths and identifiers to empty strings
     * - Clearing tools usage history
     */
    void Clear();

    /**
     * @brief Add a new sample set (source or target group) to the dataset
     *
     * Adds a collection of elemental profiles under a group name.
     * Typical group names are source identifiers (e.g., "Agricultural soil",
     * "Road dust") or the target group name (e.g., "Stream sediment").
     *
     * @param name Group name for this sample set
     * @param elemental_profile_set Collection of elemental profiles for this group (default: empty)
     * @return Pointer to the added profile set, or nullptr if name already exists
     */
    Elemental_Profile_Set* AppendSampleSet(
        const string& name,
        const Elemental_Profile_Set& elemental_profile_set = Elemental_Profile_Set()
    );

    /**
    * @brief Retrieves the names of all source groups (excluding target)
    *
    * Returns a list of source group names, which are the pollution/emission
    * sources being apportioned in the CMB analysis.
    *
    * @return Vector of source group names
    *
    * @note Target group is filtered out
    */
    vector<string> SourceGroupNames() const;

    /**
     * @brief Sets the currently selected target sample for analysis
     *
     * Specifies which sample from the target group will be used as the receptor
     * in CMB calculations. Validates that the sample exists before setting.
     *
     * @param sample_name Name of the target sample to analyze
     * @return true if sample exists and was selected, false if sample not found
     *
     * @note Must be called before InitializeParametersAndObservations()
     */
    bool SetSelectedTargetSample(const string& sample_name);

    /**
     * @brief Get a sample set (source or target group) by name
     *
     * Retrieves a pointer to the Elemental_Profile_Set for a specific group.
     * Returns nullptr if the group does not exist.
     *
     * @param name Group name to retrieve
     * @return Pointer to the profile set, or nullptr if not found
     */
    Elemental_Profile_Set* GetSampleSet(const string& name);

    /**
     * @brief Sets the target/sink group designation
     *
     * Specifies which group in the dataset represents the target samples
     * (receptor sites to be unmixed via source apportionment).
     *
     * @param targroup Name of the target group
     * @return Always returns true
     */
    bool SetTargetGroup(const string& targroup);

    /**
     * @brief Retrieves the name of the target group
     *
     * @return Name of the target group, or empty string if not set
     */
    string GetTargetGroup();

    // ========== Dataset Creation and Filtering ==========

    /**
     * @brief Create a corrected copy of the dataset for a specific target sample
     *
     * Creates a new SourceSinkData object with:
     * - Only samples marked for inclusion in analysis
     * - Optional organic matter and particle size corrections applied
     * - Element information filtered based on analysis flags
     * - All distributions populated and assigned
     *
     * This method is typically used to prepare data for source apportionment
     * of a specific target sample.
     *
     * @param target Name of the target sample to analyze
     * @param omnsizecorrect If true, apply OM and size corrections to source profiles
     * @param elementinfo Element metadata for filtering (nullptr to use internal metadata)
     * @return New SourceSinkData object ready for analysis
     */
    SourceSinkData CreateCorrectedDataset(
        const string& target,
        bool omnsizecorrect,
        map<string, element_information>* elementinfo
    );

    /**
     * @brief Create a corrected and filtered copy of the dataset
     *
     * Creates a new SourceSinkData object with optional filtering and corrections:
     * - Can exclude samples not marked for analysis
     * - Can exclude elements not marked for analysis
     * - Can apply organic matter and particle size corrections
     * - Target group is never corrected (only source groups)
     *
     * This is the main method for preparing data before analysis, allowing
     * fine control over which samples/elements to include and whether to
     * apply normalizations.
     *
     * @param exclude_samples If true, exclude samples not marked for analysis
     * @param exclude_elements If true, exclude elements not marked for analysis
     * @param omnsizecorrect If true, apply OM and particle size corrections to sources
     * @param target Target sample name (empty to use current selected_target_sample_)
     * @return New corrected and filtered SourceSinkData object
     */
    SourceSinkData CreateCorrectedAndFilteredDataset(
        bool exclude_samples,
        bool exclude_elements,
        bool omnsizecorrect,
        const string& target = ""
    ) const;

    /**
     * @brief Extract only chemical elements (and optionally isotopes)
     *
     * Creates a new SourceSinkData containing only elements with role
     * 'element' (and optionally 'isotope'), excluding particle size,
     * organic matter, and other non-element constituents.
     *
     * @param isotopes If true, also include isotope ratios
     * @return New SourceSinkData with only chemical elements
     */
    SourceSinkData ExtractChemicalElements(bool isotopes) const;

    /**
     * @brief Applies Box-Cox transformation to all source groups for normalization
     *
     * Transforms elemental concentrations using the Box-Cox power transformation
     * to improve normality of distributions. This is useful for statistical analyses
     * that assume normally distributed data (e.g., t-tests, ANOVA, discriminant analysis).
     *
     * Box-Cox transformation: y = (x^λ - 1) / λ  (or ln(x) if λ = 0)
     *
     * The optimal λ parameter for each element is determined by maximum likelihood
     * estimation to best achieve normality.
     *
     * @param calculate_optimal_lambda If true, compute optimal λ for each element;
     *                                 if false, use default transformation
     *
     * @return New SourceSinkData object with transformed concentrations
     *
     * @note Target group is not transformed (only sources)
     * @note Element distributions are recalculated after transformation
     * @note Original data is preserved (returns new object)
     *
     * @see OptimalBoxCoxParameters() for λ parameter computation
     */
    SourceSinkData BoxCoxTransformed(bool calculate_optimal_lambda = false);

    // ========== Element Information Management ==========

    /**
     * @brief Retrieves pointer to the element information map
     *
     * Provides access to element metadata including roles, standard ratios,
     * base elements, and inclusion flags for all elements in the dataset.
     *
     * @return Pointer to element information map
     */
    map<string, element_information>* GetElementInformation();

    /**
     * @brief Sets element inclusion based on a specified list
     *
     * First excludes all elements from analysis, then includes only those
     * specified in the provided list. Useful for restricting analysis to
     * a subset of elements (e.g., elements with sufficient data quality).
     *
     * @param elements Vector of element names to include in analysis
     *
     * @note All elements not in the list are excluded
     * @note Elements in the list that don't exist are silently ignored
     * @note Does not affect element roles, only the inclusion flag
     */
    void IncludeExcludeElementsBasedOn(const vector<string>& elements);

    /**
     * @brief Get the fitted distribution for a specific element at dataset level
     *
     * Returns a pointer to the Distribution object fitted to all data for
     * this element (across all groups). This is the distribution used for
     * likelihood calculations in MCMC.
     *
     * @param element_name Name of the element
     * @return Pointer to fitted Distribution, or nullptr if element not found
     */
    Distribution* GetFittedDistribution(const string& element_name);
    
    // ========== OM/Size Corrections ==========

    /**
     * @brief Performs multiple linear regression of elements vs OM and particle size
     *
     * Computes regression models for all sample groups to quantify how elemental
     * concentrations vary with organic matter (OM) content and particle size.
     * These models are used to correct elemental profiles for OM/size variations.
     *
     * Regression forms:
     * - Linear: C_corrected = C + β₁(OM_ref - OM) + β₂(Size_ref - Size)
     * - Multiplicative: C_corrected = C × (OM_ref/OM)^β₁ × (Size_ref/Size)^β₂
     *
     * @param om Name of the organic matter constituent (e.g., "OC", "OM")
     * @param particle_size Name of the particle size constituent (e.g., "D50", "PM2.5")
     * @param form Regression form (linear or multiplicative)
     * @param p_value_threshold Significance threshold for including predictors (default: 0.05)
     *
     * @return true if regressions were computed successfully
     *
     * @note Stores OM and size constituent names for later correction
     * @note Regressions are computed for all groups (sources and target)
     * @note Predictors with p > threshold are excluded from the model
     *
     * @see GetMLRResults() to retrieve regression results
     */
    bool PerformRegressionVsOMAndSize(
        const string& om,
        const string& particle_size,
        regression_form form,
        const double& p_value_threshold = 0.05
    );

    /**
     * @brief Retrieves the names of OM and particle size constituents
     *
     * Returns a vector containing the names of the organic matter and
     * particle size constituents set via PerformRegressionVsOMAndSize().
     *
     * @return Vector with [0]=OM constituent name, [1]=size constituent name
     */
    vector<string> OMandSizeConstituents();

    /**
     * @brief Retrieves multiple linear regression results for all sample groups
     *
     * Collects the organic matter (OM) and particle size regression models for
     * each source group. These regressions are used to correct elemental
     * concentrations for variations in OM content and particle size distribution.
     *
     * @return Vector of ResultItems, one per sample group, containing MLR models
     *
     * @note Each ResultItem is configured for table display
     * @note Includes both source groups and target group
     * @note Regressions must be computed first via PerformRegressionVsOMAndSize()
     */
    vector<ResultItem> GetMLRResults();

    /**
      * @brief Retrieves the name of the first organic matter constituent
      *
      * Searches element information for the first element designated as organic
      * carbon/matter.
      *
      * @return Name of first OM constituent, or empty string if none found
      *
      * @note Returns first match only
      */
    string FirstOMConstituent();

    /**
     * @brief Retrieves the name of the first particle size constituent
     *
     * Searches element information for the first element designated as particle size.
     *
     * @return Name of first size constituent, or empty string if none found
     *
     * @note Returns first match only
     */
    string FirstSizeConstituent();

    /**
     * @brief Sets the names of OM and particle size constituents
     *
     * @param _omconstituent Name of organic matter constituent
     * @param _sizeconsituent Name of particle size constituent
     */
    void SetOMandSizeConstituents(const string& _omconstituent, const string& _sizeconsituent);

    /**
     * @brief Sets the names of OM and particle size constituents from a vector
     *
     * @param _omsizeconstituents Vector with [0]=OM name, [1]=size name
     */
    void SetOMandSizeConstituents(const vector<string>& _omsizeconstituents);

    // ========== Outlier Analysis ==========

    /**
     * @brief Performs outlier detection on all source groups
     *
     * Applies Box-Cox transformation and identifies outliers based on standardized
     * residuals. Samples with standardized values outside the threshold range are
     * flagged as outliers in their notes.
     *
     * Detection method:
     * 1. Box-Cox transform each element's distribution
     * 2. Calculate z-scores: z = (x_transformed - μ) / σ
     * 3. Flag outliers: z < lower_threshold OR z > upper_threshold
     *
     * @param lower_threshold Lower bound for standardized values (e.g., -3.0)
     * @param upper_threshold Upper bound for standardized values (e.g., +3.0)
     *
     * @note Only analyzes source groups (target group excluded)
     * @note Outliers are flagged in sample notes, not removed
     * @note Uses Box-Cox transformation for normalization
     */
    void OutlierAnalysisForAll(const double& lower_threshold = -3, const double& upper_threshold = 3);

    // ========== Validation Tests ==========

    /**
     * @brief Checks for zero or negative concentration values across all sources
     *
     * Scans all source groups for elements with zero or negative concentrations,
     * which are problematic for log-normal distributions and certain statistical
     * analyses. Returns detailed error messages identifying problematic elements
     * and their source groups.
     *
     * @return Vector of error messages describing zero/negative values found
     *
     * @note Returns empty vector if no issues found
     * @note Only checks source groups (target excluded)
     */
    vector<string> NegativeValueCheck();

    /**
     * @brief Performs bracket test to check if target concentrations fall within source ranges
     *
     * Tests whether each element's concentration in the target sample falls within
     * the range (min to max) observed across all source samples. Elements outside
     * source ranges indicate potential issues: missing sources, measurement errors,
     * or non-conservative behavior.
     *
     * Test criteria for each element:
     * - Pass (0): Target concentration within [min_sources, max_sources]
     * - Fail (1): Target concentration outside source ranges
     *
     * @param target_sample Name of the target sample to test
     * @param correct_based_on_om_n_size If true, apply OM/size corrections before testing
     *
     * @return CMBVector of pass/fail flags (0 = pass, 1 = fail) for each element
     *
     * @note Flags are labeled with element names
     * @note Failed elements are documented in target sample notes
     * @note Test assumes conservative mixing (no gains/losses)
     */
    CMBVector BracketTest(const string& target_sample, bool correct_based_on_om_n_size);

    /**
     * @brief Performs bracket test on all target samples
     *
     * Applies the bracket test to every sample in the target group, producing a
     * matrix showing which elements in which samples fall outside source concentration
     * ranges.
     *
     * Matrix structure:
     * - Rows: Elements
     * - Columns: Target samples
     * - Values: 0 = pass (within range), 1 = fail (outside range)
     *
     * @param correct_based_on_om_n_size If true, apply OM/size corrections before testing
     * @param exclude_elements If true, filter out elements marked for exclusion
     * @param exclude_samples If true, filter out samples marked for exclusion
     *
     * @return CMBMatrix of bracket test results for all target samples
     *
     * @note Each target sample is tested independently
     * @note Matrix dimensions: (num_elements × num_target_samples)
     */
    CMBMatrix BracketTest(bool correct_based_on_om_n_size, bool exclude_elements, bool exclude_samples);

    // ========== Statistical Analysis - Discriminant Function Analysis ==========

    /**
     * @brief Performs discriminant function analysis for all source groups
     *
     * Conducts pairwise discriminant function analysis (DFA) comparing each source
     * group against all others combined. DFA identifies the linear combination of
     * elemental concentrations that best separates each source from the rest,
     * which helps assess source uniqueness and potential confounding.
     *
     * For each source, computes:
     * - Wilks' Lambda (measure of group separation)
     * - F-test p-value (significance of separation)
     * - Discriminant p-value (multivariate normality test)
     * - Eigenvector (discriminant function coefficients)
     * - Projected samples (scores along discriminant axis)
     *
     * @return DFA_result containing statistics and projections for all sources
     *
     * @note Target group is excluded from analysis
     * @note Each source is compared against all others combined ("one vs rest")
     * @note Lower Wilks' Lambda indicates better separation
     */
    DFA_result DiscriminantFunctionAnalysis();

    /**
     * @brief Performs discriminant function analysis between two specific sources
     *
     * Conducts pairwise DFA to assess how well two source groups can be
     * distinguished based on their elemental compositions.
     *
     * @param source1 Name of first source group
     * @param source2 Name of second source group
     *
     * @return DFA_result containing separation statistics and discriminant function
     *
     * @note Creates temporary dataset with only the two specified sources
     * @note Returns empty result if eigenvector computation fails
     */
    DFA_result DiscriminantFunctionAnalysis(const string& source1, const string& source2);

    /**
     * @brief Performs discriminant function analysis for one source vs all others
     *
     * Conducts DFA comparing a single source group against all other sources
     * combined ("one vs rest" comparison).
     *
     * @param source1 Name of source group to test
     *
     * @return DFA_result containing separation statistics and discriminant function
     *
     * @note All other source groups are pooled into an "Others" category
     */
    DFA_result DiscriminantFunctionAnalysis(const string& source1);

    /**
     * @brief Performs stepwise discriminant analysis between two specific sources
     *
     * Iteratively selects elements that best discriminate between two source groups
     * using a forward selection procedure. At each step, adds the element that most
     * improves group separation (lowest p-value).
     *
     * @param source1 Name of first source group
     * @param source2 Name of second source group
     *
     * @return Vector of 3 CMBVectors containing selection order with:
     *         [0] = p-values at each step
     *         [1] = Wilks' Lambda at each step
     *         [2] = F-test p-values at each step
     *
     * @note Elements ordered by discriminating power (best first)
     * @note Progress updates sent to rtw_ if available
     */
    vector<CMBVector> StepwiseDiscriminantFunctionAnalysis(
        const string& source1,
        const string& source2
    );

    /**
     * @brief Performs stepwise discriminant analysis across all source groups
     *
     * Iteratively selects elements that best discriminate among all source groups
     * using forward selection.
     *
     * @return Vector of 3 CMBVectors containing selection order with:
     *         [0] = p-values at each step
     *         [1] = Wilks' Lambda at each step
     *         [2] = F-test p-values at each step
     *
     * @note Uses multi-group DFA statistics
     */
    vector<CMBVector> StepwiseDiscriminantFunctionAnalysis();

    /**
     * @brief Performs stepwise discriminant analysis for one source vs all others
     *
     * Iteratively selects elements that best discriminate one source from all
     * other sources combined using forward selection.
     *
     * @param source1 Name of source group to test against others
     *
     * @return Vector of 3 CMBVectors containing selection order
     *
     * @note Other sources pooled into "Others" group
     */
    vector<CMBVector> StepwiseDiscriminantFunctionAnalysis(const string& source1);

    // ========== Statistical Analysis - Differentiation Power ==========

    /**
     * @brief Computes differentiation power for all source pairs
     *
     * Performs pairwise differentiation power analysis for all combinations of
     * source groups, generating a comprehensive matrix of source separability.
     *
     * Formula: D = 2 × |μ₁ - μ₂| / (σ₁ + σ₂)
     *
     * Interpretation:
     * - D > 2: Strong differentiation
     * - D ≈ 1: Moderate differentiation
     * - D < 0.5: Weak differentiation
     *
     * @param use_log If true, compute using log-space statistics;
     *                if false, use linear-space statistics
     * @param include_target If true, include target group in comparisons
     *
     * @return Elemental_Profile_Set with differentiation profiles for each source pair
     */
    Elemental_Profile_Set DifferentiationPower(bool use_log, bool include_target);

    /**
     * @brief Computes rank-based differentiation percentage for all source pairs
     *
     * Performs pairwise rank-based differentiation analysis for all combinations
     * of source groups. Provides a non-parametric measure of source separability.
     *
     * @param include_target If true, include target group in comparisons
     *
     * @return Elemental_Profile_Set with differentiation percentages for each source pair
     *
     * @note Values near 1.0 indicate excellent separation
     * @note Values near 0.5 indicate no separation
     */
    Elemental_Profile_Set DifferentiationPower_Percentage(bool include_target);

    /**
     * @brief Computes t-test p-values for all source pairs
     *
     * Performs pairwise t-test analysis for all combinations of source groups,
     * generating a comprehensive matrix of statistical significance values.
     *
     * @param include_target If true, include target group in comparisons
     *
     * @return Elemental_Profile_Set with p-value profiles for each source pair
     *
     * @note Lower p-values indicate statistically significant differences
     */
    Elemental_Profile_Set DifferentiationPower_P_value(bool include_target);

    /**
     * @brief Computes rank-based differentiation percentage between two sources
     *
     * Calculates the percentage of samples that can be correctly classified based
     * on element concentrations using a rank-based approach. Provides a
     * distribution-free measure of source separability.
     *
     * Method:
     * 1. Pool samples from both sources
     * 2. Rank all concentrations for each element
     * 3. Count correct classifications (Source1 samples with low ranks +
     *    Source2 samples with high ranks, or vice versa)
     * 4. Return classification success percentage
     *
     * @param source1 Name of first source group
     * @param source2 Name of second source group
     *
     * @return Elemental_Profile containing classification percentages (0-1) for each element
     *
     * @note Values near 1.0 indicate excellent separation
     * @note Values near 0.5 indicate no separation (random classification)
     * @note Non-parametric (no distributional assumptions)
     *
     * @see DifferentiationPower_Percentage(bool) for all pairwise comparisons
     */
    Elemental_Profile DifferentiationPower_Percentage(const string& source1, const string& source2);

    /**
     * @brief Computes differentiation power metric between two sources
     *
     * Calculates a standardized measure of how well each element differentiates
     * between two sources. The metric quantifies separation in units of pooled
     * standard deviations.
     *
     * Formula: D = 2 × |μ₁ - μ₂| / (σ₁ + σ₂)
     *
     * Interpretation:
     * - D > 2: Strong differentiation (means differ by >1 pooled std dev)
     * - D ≈ 1: Moderate differentiation
     * - D < 0.5: Weak differentiation (substantial overlap)
     *
     * @param source1 Name of first source group
     * @param source2 Name of second source group
     * @param use_log If true, compute using log-space statistics;
     *                if false, use linear-space statistics
     *
     * @return Elemental_Profile containing differentiation power for each element
     *
     * @note Higher values indicate better discrimination ability
     * @note Complementary to p-values (considers effect size, not just significance)
     *
     * @see DifferentiationPower(bool, bool) for all pairwise comparisons
     */
    Elemental_Profile DifferentiationPower(const string& source1, const string& source2, bool use_log);

    /**
     * @brief Computes t-test p-values for element-wise differences between two sources
     *
     * Performs independent two-sample t-tests for each element to assess whether
     * concentrations differ significantly between two source groups. Lower p-values
     * indicate stronger evidence that the sources differ for that element.
     *
     * T-statistic: t = (μ₁ - μ₂) / √(σ₁²/n₁ + σ₂²/n₂)
     *
     * @param source1 Name of first source group
     * @param source2 Name of second source group
     * @param use_log If true, perform test on log-transformed concentrations;
     *                if false, use linear-space concentrations
     *
     * @return Elemental_Profile containing two-tailed p-values for each element
     *
     * @note P-values are two-tailed (tests for any difference, not directional)
     * @note Lower p-values indicate better source differentiation
     * @note Log-space testing appropriate for log-normally distributed data
     *
     * @see DifferentiationPower_P_value() for all pairwise comparisons
     */
    Elemental_Profile t_TestPValue(const string& source1, const string& source2, bool use_log);

    // ========== Statistical Analysis - ANOVA ==========

    /**
     * @brief Performs one-way ANOVA for all elements across source groups
     *
     * Conducts analysis of variance to test whether mean concentrations differ
     * significantly among source groups for each element.
     *
     * Null hypothesis: μ₁ = μ₂ = ... = μₖ (all source means equal)
     *
     * @param use_log If true, perform ANOVA on log-transformed concentrations;
     *                if false, use linear-space concentrations
     *
     * @return CMBVector of p-values for each element, labeled with element names
     *
     * @note Lower p-values indicate significant differences among sources
     * @note Target group is excluded from analysis
     */
    CMBVector ANOVA(bool use_log);

    /**
     * @brief Performs one-way ANOVA for a single element across source groups
     *
     * Conducts detailed analysis of variance decomposing total variance into
     * between-group and within-group components.
     *
     * ANOVA decomposition:
     * - SST (Total) = SSB + SSW
     * - SSB (Between-group) = Σ n_i(μ_i - μ_grand)²
     * - SSW (Within-group) = Σ Σ (x_ij - μ_i)²
     * - F-statistic = MSB / MSW
     *
     * @param element Name of the element to analyze
     * @param use_log If true, perform ANOVA on log-transformed concentrations
     *
     * @return ANOVA_info structure containing SST, SSB, SSW, MSB, MSW, F, and p-value
     */
    ANOVA_info ANOVA(const string& element, bool use_log);

    // ========== Parameter and Observation Setup ==========

    /**
     * @brief Get the name of a parameter by its index
     *
     * Retrieves the descriptive name of an optimization parameter
     * (e.g., "Agricultural_contribution", "Urban_Al_mu") by its
     * position in the parameters vector.
     *
     * @param index Parameter index (0-based)
     * @return Parameter name, or empty string if index invalid
     */
    string GetParameterName(int index) const;

    /**
     * @brief Initialize parameters and observations for MCMC optimization
     *
     * Sets up the complete parameter space and observation vector for Bayesian
     * inference of source contributions and/or source profiles. The exact
     * parameters created depend on the estimation mode:
     *
     * - only_contributions: Only source contribution parameters
     * - elemental_profile_and_contribution: Contributions + mu/sigma for each source-element
     * - source_elemental_profiles_based_on_source_data: Only source profile parameters
     *
     * @param targetsamplename Name of the target sample to unmix
     * @param est_mode Estimation mode controlling which parameters to optimize
     * @return true if successful, false if data not loaded
     *
     * @note Must be called before running MCMC or optimization
     */
    bool InitializeParametersAndObservations(
        const string& targetsamplename,
        estimation_mode est_mode = estimation_mode::elemental_profile_and_contribution
    );

    /**
     * @brief Sets the estimation mode for parameter optimization
     *
     * Specifies which parameters will be optimized during inference:
     * - only_contributions: Estimate only source contributions (fixed source profiles)
     * - elemental_profile_and_contribution: Estimate both contributions and source profiles
     * - source_elemental_profiles_based_on_source_data: Estimate only source profiles
     *
     * @param est_mode Estimation mode to use
     */
    void SetParameterEstimationMode(estimation_mode est_mode);

    /**
     * @brief Sets a parameter value and updates corresponding model components
     *
     * Updates a parameter in the parameter vector and synchronizes the change with
     * the appropriate model component (contributions, distribution parameters, or
     * error terms). The parameter vector layout depends on the estimation mode.
     *
     * Parameter Vector Layout (full mode: elemental_profile_and_contribution):
     *   [0 to n-2]:                    Contribution parameters (n-1 sources)
     *   [n-1 to end of element μ]:     Element μ parameters
     *   [element μ to end of isotope μ]: Isotope μ parameters
     *   [isotope μ to end of element σ]: Element σ parameters
     *   [element σ to end of isotope σ]: Isotope σ parameters
     *   [end-1]:                       Error std dev for elements
     *   [end]:                         Error std dev for isotopes
     *
     * @param index Parameter index in the parameters_ vector
     * @param value New parameter value
     * @return true if parameter was successfully updated, false if index is invalid
     *
     * @note Automatically updates last contribution to maintain sum constraint
     * @note Validates that standard deviations are non-negative
     */
    bool SetParameterValue(size_t index, double value);

    /**
     * @brief Sets the progress window for displaying optimization progress
     *
     * Assigns a progress window that will receive updates during long-running
     * operations like MCMC sampling, bootstrap analysis, or batch processing.
     *
     * @param _rtw Pointer to ProgressWindow object (nullptr to disable progress display)
     */
    void SetProgressWindow(ProgressWindow* _rtw);

    

    // ========== Optimization - Levenberg-Marquardt ==========

    /**
     * @brief Solves for optimal source contributions using the Levenberg-Marquardt algorithm
     *
     * Iteratively optimizes source contributions to minimize the residual between predicted
     * and observed elemental/isotopic compositions. The algorithm adaptively adjusts the
     * damping parameter based on convergence behavior.
     *
     * Convergence criteria (any of):
     * - Residual norm < tolerance (1e-10)
     * - Parameter change norm < tolerance (1e-10)
     * - Maximum iterations reached (1000)
     *
     * @param trans Parameterization method:
     *        - transformation::linear: Direct contribution values with constraint Σc_i = 1
     *        - transformation::softmax: Unconstrained parameters transformed via softmax
     *
     * @return bool Currently always returns false (legacy)
     *
     * @note Updates internal state with optimized contributions
     * @note Progress displayed in rtw_ if set
     */
    bool SolveLevenberg_Marquardt(transformation trans = transformation::linear);

    /**
     * @brief Solves CMB model for all target samples using Levenberg-Marquardt
     *
     * Performs batch source apportionment on all samples in the target group,
     * solving the CMB model via Levenberg-Marquardt optimization for each sample.
     *
     * @param transform Transformation type (linear or softmax) for contributions
     * @param apply_om_size_correction If true, apply OM and size corrections before solving
     * @param negative_elements [out] Map of sample names to vectors of elements
     *                          with negative values
     *
     * @return CMBTimeSeriesSet containing contribution estimates for all valid samples
     *
     * @note Samples with negative values are skipped and recorded
     * @note Progress updates sent to rtw_ if available
     */
    CMBTimeSeriesSet LM_Batch(
        transformation transform,
        bool apply_om_size_correction,
        map<string, vector<string>>& negative_elements
    );

    /**
     * @brief Retrieves predicted values for all observations
     *
     * Collects the predicted values from all observation objects for comparison
     * with actual observations.
     *
     * @return Vector of predicted values matching the observations vector
     */
    CVector GetPredictedValues();

    /**
     * @brief Performs one gradient ascent step with adaptive step size
     *
     * Executes a single iteration of gradient ascent optimization on the log-likelihood
     * function using an adaptive step size (distance_coeff_).
     *
     * Algorithm:
     * 1. Compute gradient direction at current parameters
     * 2. Try step sizes: distance_coeff_ and 2×distance_coeff_
     * 3. Accept step that improves likelihood
     * 4. Adjust step size for next iteration
     *
     * @param estmode Estimation mode determining which likelihood components to include
     * @return Updated parameter vector after the gradient step
     *
     * @note Modifies distance_coeff_ member variable for next iteration
     */
    CVector GradientUpdate(estimation_mode estmode = estimation_mode::elemental_profile_and_contribution);

    // ========== MCMC Analysis ==========

    /**
     * @brief Performs Markov Chain Monte Carlo analysis for Bayesian source apportionment
     *
     * Conducts full Bayesian inference using MCMC sampling to estimate posterior
     * distributions of source contributions and model parameters. Generates comprehensive
     * uncertainty quantification including credible intervals, posterior distributions,
     * and predicted concentrations.
     *
     * @param target_sample Name of target sample to apportion
     * @param arguments Map of MCMC settings (number of samples, chains, burnin, etc.)
     * @param mcmc Pointer to MCMC sampler object
     * @param progress_window Pointer to progress window for updates
     * @param working_folder Base directory for output files
     *
     * @return Results object containing all MCMC outputs and credible intervals
     *
     * @note Burnin samples excluded from posterior statistics
     * @note Last contribution computed from sum constraint
     */
    Results MCMC(
        const string& target_sample,
        map<string, string> arguments,
        CMCMC<SourceSinkData>* mcmc,
        ProgressWindow* progress_window,
        const string& working_folder
    );

    /**
     * @brief Performs batch MCMC analysis on all target samples
     *
     * Conducts full Bayesian MCMC analysis for every sample in the target group,
     * generating comprehensive results for each sample and summarizing credible
     * intervals in a matrix.
     *
     * Output matrix structure:
     * - Rows: Target samples
     * - Columns (4 per source): [source]_low, [source]_high, [source]_median, [source]_mean
     *
     * @param arguments Map of MCMC settings
     * @param mcmc Pointer to MCMC sampler object
     * @param progress_window Pointer to progress window for updates
     * @param working_folder Base directory for output folders
     *
     * @return CMBMatrix containing credible interval statistics for all samples
     *
     * @note Creates subdirectory for each target sample
     * @note All ResultItems saved as text files
     */
    CMBMatrix MCMC_Batch(
        map<string, string> arguments,
        CMCMC<SourceSinkData>* mcmc,
        ProgressWindow* progress_window,
        const string& working_folder
    );

    // --- Likelihood and Objective Functions ---

    /**
     * @brief Returns the objective function value for optimization algorithms
     *
     * Computes the negative log-likelihood based on the current parameter values
     * and estimation mode. Used by optimization algorithms where minimizing -log(L)
     * is equivalent to maximizing likelihood L.
     *
     * @return Negative log-likelihood value (lower values indicate better fit)
     */
    double GetObjectiveFunctionValue();

    /**
     * @brief Calculates the total log-likelihood for Bayesian source apportionment
     *
     * Computes the posterior log-likelihood by summing multiple components based
     * on the estimation mode. Components include source data likelihood, observation
     * likelihood (elements and isotopes), and contribution priors.
     *
     * @param est_mode Estimation mode determining which likelihood terms to include
     * @return Total log-likelihood (higher values indicate better model fit)
     */
    double LogLikelihood(estimation_mode est_mode = estimation_mode::elemental_profile_and_contribution);


    // ========== Bootstrap and Validation ==========

    /**
     * @brief Performs bootstrap uncertainty analysis on source contributions
     *
     * Conducts bootstrap resampling to estimate uncertainty in source contribution
     * estimates. Each bootstrap iteration randomly excludes a percentage of source
     * samples, re-solves the CMB model, and records the resulting contributions.
     *
     * @param percentage Percentage of source samples to exclude per iteration (0-100)
     * @param num_iterations Number of bootstrap iterations
     * @param target_sample Name of target sample to apportion
     * @param use_softmax If true, use softmax transformation for contributions
     *
     * @return CMBTimeSeriesSet containing contribution estimates from all iterations
     *
     * @note Progress updates sent to rtw_ if available
     */
    CMBTimeSeriesSet BootStrap(
        const double& percentage,
        unsigned int num_iterations,
        string target_sample,
        bool use_softmax
    );

    /**
     * @brief Performs bootstrap analysis and generates comprehensive uncertainty results
     *
     * Conducts bootstrap resampling with full statistical analysis including:
     * - Time series of contribution estimates
     * - Posterior distributions of contributions
     * - 95% credible intervals (2.5th to 97.5th percentiles)
     * - Mean and median contribution estimates
     *
     * @param results Pointer to Results object to populate with analysis outputs
     * @param percentage Percentage of source samples to exclude per iteration (0-100)
     * @param num_iterations Number of bootstrap iterations
     * @param target_sample Name of target sample to apportion
     * @param use_softmax If true, use softmax transformation
     *
     * @return true if analysis completed successfully
     *
     * @note Generates stacked bar chart for contributions (if iterations ≤ 100)
     * @note Computes posterior distributions with 100 bins
     */
    bool BootStrap(
        Results* results,
        const double& percentage,
        unsigned int num_iterations,
        string target_sample,
        bool use_softmax
    );

    /**
     * @brief Performs leave-one-out validation on a source group
     *
     * Tests source apportionment accuracy by iteratively treating each sample
     * from a source group as an unknown target, solving the CMB model, and
     * comparing estimated contributions against the known true source.
     *
     * Ideal result: Sample shows ~100% contribution from its true source
     *
     * @param source_group Name of source group to validate
     * @param use_softmax If true, use softmax transformation for contributions
     * @param apply_om_size_correction If true, apply OM and size corrections
     *
     * @return CMBTimeSeriesSet containing contribution estimates for each sample
     *
     * @note Samples with negative values after correction are skipped
     * @note Progress updates sent to rtw_ if available
     */
    CMBTimeSeriesSet VerifySource(
        const string& source_group,
        bool use_softmax,
        bool apply_om_size_correction
    );

    // ========== Results Retrieval ==========

    /**
     * @brief Packages source contributions into a ResultItem for output
     *
     * Creates a ResultItem containing the current source contribution estimates,
     * formatted for export or display.
     *
     * @return ResultItem of type 'contribution' containing source fractions
     *
     * @note Creates new Contribution object on heap (managed by ResultItem)
     */
    ResultItem GetContribution();

    /**
     * @brief Retrieves the observed elemental concentrations for the selected target sample
     *
     * Extracts the measured elemental composition of the currently selected target
     * sample. These are the observed values that the model attempts to reproduce.
     *
     * @return ResultItem containing the observed Elemental_Profile for chemical elements
     *
     * @note Only includes chemical elements, not isotopes
     */
    ResultItem GetObservedElementalProfile();

    /**
     * @brief Retrieves the observed isotope delta values for the selected target sample
     *
     * Extracts the measured isotopic composition (δ values in ‰) of the currently
     * selected target sample.
     *
     * @return ResultItem containing the observed Elemental_Profile with isotope δ values
     *
     * @note Only includes isotopes, not chemical elements
     * @note Values are in delta notation (‰)
     */
    ResultItem GetObservedElementalProfile_Isotope();

    /**
     * @brief Generates predicted elemental concentrations for the target sample
     *
     * Computes the model's predicted elemental composition using the current
     * source contributions and profiles.
     *
     * @param param_mode Controls whether to use parametric or empirical source means
     *
     * @return ResultItem containing the predicted Elemental_Profile
     *
     * @note Only includes chemical elements, not isotopes
     */
    ResultItem GetPredictedElementalProfile(
        parameter_mode param_mode = parameter_mode::based_on_fitted_distribution
    );

    /**
     * @brief Generates predicted isotope delta values for the target sample
     *
     * Computes the model's predicted isotopic composition (δ values in ‰) using
     * current source contributions and isotope profiles.
     *
     * @param param_mode Controls whether to use parametric or empirical source means
     *
     * @return ResultItem containing the predicted Elemental_Profile with isotope δ values
     *
     * @note Only includes isotopes, not chemical elements
     * @note Values are in delta notation (‰)
     */
    ResultItem GetPredictedElementalProfile_Isotope(
        parameter_mode param_mode = parameter_mode::based_on_fitted_distribution
    );

    /**
     * @brief Creates a comparison of observed vs modeled elemental profiles
     *
     * Generates a profile set containing both the observed target sample
     * composition and the model's predicted composition for side-by-side
     * comparison.
     *
     * @param param_mode Controls whether to use parametric or empirical source means
     *
     * @return ResultItem containing Elemental_Profile_Set with "Observed" and "Modeled" profiles
     *
     * @note Only includes chemical elements, not isotopes
     */
    ResultItem GetObservedvsModeledElementalProfile(
        parameter_mode param_mode = parameter_mode::based_on_fitted_distribution
    );

    /**
     * @brief Creates a comparison of observed vs modeled isotope delta values
     *
     * Generates a profile set containing both the observed target sample
     * isotopic composition and the model's predicted isotope delta values.
     *
     * @param param_mode Controls whether to use parametric or empirical source means
     *
     * @return ResultItem containing Elemental_Profile_Set with "Observed" and "Modeled" isotope profiles
     *
     * @note Only includes isotopes, not chemical elements
     * @note All values are in delta notation (‰)
     */
    ResultItem GetObservedvsModeledElementalProfile_Isotope(
        parameter_mode param_mode = parameter_mode::based_on_fitted_distribution
    );

    /**
     * @brief Computes estimated mean concentrations for all source elements
     *
     * Calculates the mean elemental concentrations for each source group using
     * the current estimated distributions.
     *
     * @return ResultItem containing Elemental_Profile_Set with mean profiles for each source
     *
     * @note Only includes source groups (target excluded)
     * @note Uses estimated distributions (updated during optimization/MCMC)
     */
    ResultItem GetCalculatedElementMeans();

    /**
     * @brief Computes estimated standard deviations for all source elements
     *
     * Calculates the standard deviations of elemental concentrations for each
     * source group using the current estimated distributions.
     *
     * @return ResultItem containing Elemental_Profile_Set with std dev profiles for each source
     *
     * @note Different calculations for normal vs log-normal distributions
     */
    ResultItem GetCalculatedElementSigma();

    /**
     * @brief Computes μ parameters from fitted log-normal distributions for source elements
     *
     * Calculates the μ (log-space mean) parameter for each element in each source
     * group based on the fitted log-normal distributions from source sample data.
     *
     * @return ResultItem containing Elemental_Profile_Set with μ values for each source
     *
     * @note Uses CalculateMeanLog() - computed from fitted distribution
     */
    ResultItem GetCalculatedElementMu();

    /**
     * @brief Retrieves estimated μ parameters from Bayesian inference for source elements
     *
     * Returns the inferred μ (log-space mean) parameters for each element in each
     * source group as estimated during MCMC sampling or optimization.
     *
     * @return ResultItem containing Elemental_Profile_Set with inferred μ values
     *
     * @note Uses GetEstimatedMu() - updated during MCMC/optimization
     */
    ResultItem GetEstimatedElementMu();

    /**
     * @brief Computes actual mean concentrations from estimated log-normal parameters
     *
     * Calculates the actual concentration means for each element in each source
     * using the estimated μ and σ parameters from Bayesian inference.
     *
     * Formula: Mean = exp(μ + σ²/2)
     *
     * @return ResultItem containing Elemental_Profile_Set with mean concentrations
     */
    ResultItem GetEstimatedElementMean();

    /**
     * @brief Retrieves estimated σ parameters from Bayesian inference for source elements
     *
     * Returns the inferred σ (log-space standard deviation) parameters for each
     * element in each source group as estimated during MCMC sampling or optimization.
     *
     * @return ResultItem containing Elemental_Profile_Set with inferred σ values
     *
     * @note Values are in log-space (not actual concentration std dev)
     */
    ResultItem GetEstimatedElementSigma();

    /**
     * @brief Retrieves elemental profiles for all source groups
     *
     * Collects the complete elemental profile sets for each source group,
     * packaged as ResultItems for display.
     *
     * @return Vector of ResultItems, one per source group
     *
     * @note Target group is excluded
     */
    vector<ResultItem> GetSourceProfiles();

    // ========== Utility ==========

    /**
     * @brief Adds a tool name to the list of tools used in analysis
     *
     * Records that a particular analysis tool was applied to this dataset.
     * Prevents duplicate entries.
     *
     * @param tool Name of the tool used (e.g., "MCMC", "Bootstrap")
     */
    void AddtoToolsUsed(const string& tool);

    // ========== Accessors (inline for performance) ==========

    /**
     * @brief Retrieves reference to the parameters vector
     * @return Reference to parameters vector
     */
    vector<Parameter>& Parameters();

    /**
     * @brief Returns the number of parameters
     * @return Number of parameters in the optimization
     */
    size_t ParametersCount();

    /**
     * @brief Returns the number of observations
     * @return Number of observations (measured values)
     */
    size_t ObservationsCount();

    /**
     * @brief Retrieves pointer to a parameter by index
     * @param i Parameter index (0-based)
     * @return Pointer to Parameter, or nullptr if invalid index
     */
    Parameter* parameter(size_t i);

    /**
     * @brief Retrieves const pointer to a parameter by index
     * @param i Parameter index (0-based)
     * @return Const pointer to Parameter, or nullptr if invalid index
     */
    const Parameter* parameter(size_t i) const;

    /**
     * @brief Retrieves pointer to an observation by index
     * @param i Observation index (0-based)
     * @return Pointer to Observation, or nullptr if invalid index
     */
    Observation* observation(size_t i);

    /**
     * @brief Retrieves the current estimation mode
     * @return Current estimation mode setting
     */
    estimation_mode ParameterEstimationMode();

    /**
     * @brief Retrieves pointer to element distribution at dataset level
     * @param element_name Name of the element
     * @return Pointer to ConcentrationSet, or nullptr if not found
     */
    ConcentrationSet* GetElementDistribution(const string& element_name);

    /**
     * @brief Retrieves pointer to element distribution for a specific group
     * @param element_name Name of the element
     * @param sample_group Name of the source or target group
     * @return Pointer to ConcentrationSet, or nullptr if not found
     */
    ConcentrationSet* GetElementDistribution(const string& element_name, const string& sample_group);

    /**
     * @brief Retrieves pointer to element information metadata by name
     *
     * @param element_name Name of the element
     * @return Pointer to element_information, or nullptr if not found
     */
    element_information* GetElementInformation(const string& element_name);

    // --- Distribution Management ---

    /**
     * @brief Populate element distributions from all groups
     *
     * Builds concentration distributions for each element by collecting
     * all concentration values across all groups (sources and target).
     * First updates distributions within each group, then aggregates
     * them into the overall element_distributions_ map.
     *
     * @note Should be called after loading data or modifying profiles
     */
    void PopulateElementDistributions();

    /**
    * @brief Populate element information metadata
    *
    * Initializes or updates the element_information_ map with metadata
    * for all elements in the dataset. If ElementInfo is provided, copies
    * metadata from it; otherwise initializes with default values.
    *
    * Also updates the count of source sample sets (total groups minus
    * target group if present).
    *
    * @param ElementInfo Optional source of element metadata (nullptr for defaults)
    */
    void PopulateElementInformation(const map<string, element_information>* ElementInfo = nullptr);

    /**
     * @brief Assign distributions to all elements at both dataset and group levels
     *
     * For each element:
     * 1. At dataset level: Selects best-fitting distribution (normal vs lognormal)
     *    and estimates parameters from all data
     * 2. At group level: Assigns same distribution type and estimates parameters
     *    from group-specific data
     *
     * Special handling for isotopes: Always uses normal distribution regardless
     * of best-fit results.
     *
     * @note Should be called after PopulateElementDistributions()
     */
    void AssignAllDistributions();

    // --- Data Extraction and Management ---

   /**
    * @brief Count the number of elements in the dataset
    *
    * Counts elements based on their inclusion status and role. Can count either:
    * - All elements (if exclude_elements = false)
    * - Only elements marked for analysis, excluding OM, particle size, and do_not_include roles
    *
    * @param exclude_elements If true, only count elements marked for analysis with valid roles
    * @return Number of elements meeting the criteria
    */
    int CountElements(bool exclude_elements) const;

    /**
     * @brief Get all sample names within a specific group
     *
     * Returns the names of all samples (elemental profiles) within a
     * specified source or target group.
     *
     * @param group_name Name of the group to query
     * @return Vector of sample names, or empty vector if group not found
     */
    vector<string> GetSampleNames(const string& group_name) const;

    /**
     * @brief Get all group names in the dataset
     *
     * Returns the names of all sample sets (source and target groups).
     * These are the top-level keys of the SourceSinkData map.
     *
     * @return Vector of all group names
     */
    vector<string> GetGroupNames() const;

    /**
     * @brief Get all element names in the dataset
     *
     * Returns element names by examining the first sample of the first group.
     * Assumes all samples have the same elements (which should be true after
     * proper data loading).
     *
     * @return Vector of element names, or empty vector if dataset is empty
     */
    vector<string> GetElementNames() const;

    /**
     * @brief Sets inclusion flag for all elements
     *
     * Enables or disables all elements for analysis in batch.
     *
     * @param include_in_analysis If true, include all; if false, exclude all
     *
     * @note Affects all elements regardless of role
     */
    void IncludeExcludeAllElements(bool include_in_analysis);

    /**
     * @brief Retrieves an elemental profile by sample name
     *
     * Searches all groups (sources and target) for a sample with the specified
     * name and returns its elemental profile. Returns empty profile if not found.
     *
     * @param sample_name Name of the sample to retrieve
     * @return Elemental_Profile for the sample, or empty profile if not found
     *
     * @note Searches all groups sequentially
     * @note Returns copy of profile, not reference
     * @note If multiple samples have the same name, returns first match
     */
    Elemental_Profile Sample(const string& sample_name) const;

    /**
     * @brief Extract concentration data for specified samples
     *
     * Extracts elemental concentration data for a list of samples identified
     * by [group_name, sample_name] pairs. Returns data in a structured format
     * suitable for analysis.
     *
     * @param indicators Vector of [group_name, sample_name] pairs identifying samples
     * @return profiles_data structure containing element names, sample names, and concentration values
     */
    profiles_data ExtractConcentrationData(const vector<vector<string>>& indicators) const;

    /**
     * @brief Extract samples as an Elemental_Profile_Set
     *
     * Extracts specified samples and combines them into a single profile set.
     * Sample names are prefixed with their group name (e.g., "Agricultural-Sample1")
     * to ensure uniqueness when combining samples from different groups.
     *
     * @param indicators Vector of [group_name, sample_name] pairs identifying samples
     * @return Elemental_Profile_Set containing the extracted samples
     */
    Elemental_Profile_Set ExtractSamplesAsProfileSet(const vector<vector<string>>& indicators) const;

    /**
     * @brief Extract concentration data for a specific element from a group
     *
     * Retrieves all concentration values for a single element across all
     * samples within a specified group. Useful for univariate analysis
     * of a single element's distribution within a source or target group.
     *
     * @param element Element name to extract
     * @param group Group name to extract from
     * @return element_data structure containing group name, sample names, and concentration values
     */
    element_data ExtractElementConcentrations(const string& element, const string& group) const;

    /**
     * @brief Extract concentration data for a specific element from all groups
     *
     * Retrieves all concentration values for a single element organized by group.
     * Returns a map where keys are group names and values are vectors of
     * concentrations for that element within each group.
     *
     * @param element Element name to extract
     * @return Map of group_name -> vector of concentration values
     */
    map<string, vector<double>> ExtractElementDataByGroup(const string& element) const;

    /**
     * @brief Extract specific elements from source groups
     *
     * Creates a new SourceSinkData containing only the specified elements
     * from source groups. Target group is excluded. Useful for subset
     * analysis or element selection optimization.
     *
     * @param element_list List of element names to extract
     * @return New SourceSinkData with only specified elements from sources
     */
    SourceSinkData ExtractSpecificElements(const vector<string>& element_list) const;

    /**
     * @brief Extracts concentration distributions for all elements across sources
     *
     * Collects all concentration values for each element across all source samples
     * (excluding target) into ConcentrationSet objects. This aggregates data for
     * statistical analysis of elemental distributions across the entire source dataset.
     *
     * @return Map of element names to ConcentrationSet objects containing all
     *         source sample concentrations for that element
     *
     * @note Target group samples are excluded
     * @note Each ConcentrationSet contains values from all sources combined
     */
    map<string, ConcentrationSet> ExtractConcentrationSet();

    // --- Element Ordering ---

    /**
     * @brief Identifies chemical elements to be used in CMB analysis
     *
     * Scans the element information map and collects all elements marked with the
     * 'element' role. Updates constituent count and ordering vectors.
     *
     * @return Vector of element names to be included in CMB analysis
     *
     * @note Updates constituent_order_ and numberofconstituents_
     */
    vector<string> ElementsToBeUsedInCMB();

    /**
     * @brief Identifies isotopes to be used in CMB analysis
     *
     * Scans the element information map and collects all isotopes marked with the
     * 'isotope' role AND flagged for inclusion in analysis.
     *
     * @return Vector of isotope names to be included in CMB analysis
     *
     * @note Updates isotope_order_ and numberofisotopes_
     * @note Only includes isotopes where include_in_analysis = true
     */
    vector<string> IsotopesToBeUsedInCMB();

    /**
     * @brief Populates all element ordering vectors used throughout CMB analysis
     *
     * Scans the element information map and organizes elements into separate
     * ordering vectors based on their roles. These vectors define the order
     * in which elements appear in matrices, parameter vectors, and observations.
     *
     * Populated Vectors:
     * - constituent_order_: All species in the dataset
     * - element_order_: Chemical elements flagged for analysis
     * - isotope_order_: Isotopes flagged for analysis
     * - size_om_order_: Particle size and organic carbon parameters
     *
     * @note Must be called before InitializeParametersAndObservations()
     */
    void PopulateConstituentOrders();

    /**
     * @brief Retrieves the name of the currently selected target sample
     *
     * @return Name of the target sample being analyzed
     */
    string SelectedTargetSample() const;

    // --- Tools Tracking ---

    /**
     * @brief Checks if a specific analysis tool has been used
     *
     * Queries the tools_used list to determine if a particular analysis method
     * or tool has been applied to this dataset.
     *
     * @param tool_name Name of the tool to check (e.g., "MCMC", "Bootstrap")
     * @return true if tool has been used, false otherwise
     */
    bool ToolsUsed(const string& tool_name);

    // --- File I/O ---

    /**
     * @brief Get the output directory path
     *
     * Returns the path where analysis results and output files will be saved.
     *
     * @return Current output path
     */

    string GetOutputPath() const;

    /**
    * @brief Retrieves pointer to the options map
    *
    * @return Pointer to QMap containing configuration options
    */
    
    QMap<QString, double>* GetOptions();

    /**
     * @brief Set the output directory path
     *
     * Specifies the directory where analysis results and output files
     * should be saved.
     *
     * @param output_path Path to output directory
     * @return Always returns true
     */
    bool SetOutputPath(const string& output_path);

    /**
    * @brief Exports element information metadata to a JSON object
    *
    * Serializes all element metadata (role, standard ratio, base element,
    * inclusion flag) to a JSON object for saving or transmission.
    *
    * @return QJsonObject containing all element information
    */
    QJsonObject ElementInformationToJsonObject();

    /**
     * @brief Exports the list of analysis tools used to a JSON array
     *
     * Serializes the names of all statistical/analytical tools that have been
     * applied to this dataset (e.g., "MCMC", "Levenberg-Marquardt", "BoxCox").
     *
     * @return QJsonArray containing tool names as strings
     */
    QJsonArray ToolsUsedToJsonObject();

    /**
     * @brief Exports analysis options/settings to a JSON object
     *
     * Serializes all analysis configuration options (numerical parameters,
     * thresholds, flags) as key-value pairs.
     *
     * @return QJsonObject containing option names and values
     */
    QJsonObject OptionsToJsonObject();

    /**
     * @brief Deserializes the list of analysis tools from a JSON array
     *
     * @param jsonarray JSON array containing tool names
     * @return true if successfully loaded
     */
    bool ReadToolsUsedFromJsonObject(const QJsonArray& jsonarray);

    /**
     * @brief Deserializes element information metadata from a JSON object
     *
     * Loads element roles, standard ratios, base elements, and inclusion flags
     * from JSON format. Clears existing element information before loading.
     *
     * @param jsonobject JSON object containing element metadata
     * @return true if successfully loaded
     *
     * @note Clears element_information_ before loading
     */
    bool ReadElementInformationfromJsonObject(const QJsonObject& jsonobject);

    /**
     * @brief Deserializes elemental profile data from a JSON object
     *
     * Loads all sample groups and their elemental profiles from JSON format.
     * Clears existing data before loading.
     *
     * @param jsonobject JSON object containing elemental profile sets
     * @return true if successfully loaded
     *
     * @note Clears all existing data before loading
     */
    bool ReadElementDatafromJsonObject(const QJsonObject& jsonobject);

    /**
     * @brief Deserializes analysis options from a JSON object
     *
     * Loads configuration options and settings from JSON format.
     *
     * @param jsonobject JSON object containing option key-value pairs
     * @return true if successfully loaded
     */
    bool ReadOptionsfromJsonObject(const QJsonObject& jsonobject);

    /**
     * @brief Exports all elemental profile data to a JSON object
     *
     * Serializes all sample groups and their elemental profiles to JSON format.
     *
     * @return QJsonObject containing all elemental profile sets
     */
    QJsonObject ElementDataToJsonObject();

    /**
     * @brief Writes elemental profile data to a text file
     *
     * Outputs all sample groups and their elemental profiles in tab-delimited
     * text format for documentation or debugging purposes.
     *
     * @param file Pointer to an open QFile for writing
     * @return true if write operation succeeded
     *
     * @note File must be opened in write mode before calling
     */
    bool WriteDataToFile(QFile* file);

    /**
     * @brief Writes dataset to a text file
     *
     * Public interface for writing elemental profile data to a file.
     *
     * @param file Pointer to an open QFile for writing
     * @return true if write operation succeeded
     */
    bool WriteToFile(QFile* file);

    /**
     * @brief Loads complete dataset from a JSON file
     *
     * Deserializes all dataset components from a JSON file: elemental profiles,
     * element information, tools used, options, and target group designation.
     * Clears existing data before loading.
     *
     * @param fil Pointer to an open QFile for reading
     * @return true if successfully loaded
     *
     * @note Clears all existing data before loading
     */
    bool ReadFromFile(QFile* fil);
private:
    // ========== Private Data Members ==========

    // Element Metadata and Distributions
    map<string, element_information> element_information_;
    map<string, ConcentrationSet> element_distributions_;

    // Counters
    int numberofconstituents_;
    int numberofisotopes_;
    int numberofsourcesamplesets_;

    // MCMC/Optimization Data
    vector<Observation> observations_;
    vector<Parameter> parameters_;

    // File Paths
    string outputpath_;

    // Group and Sample Identifiers
    string target_group_;
    string selected_target_sample_;

    // Ordering Vectors
    vector<string> samplesetsorder_;
    vector<string> constituent_order_;
    vector<string> element_order_;
    vector<string> isotope_order_;
    vector<string> size_om_order_;

    // Analysis Settings
    estimation_mode parameter_estimation_mode_;
    string omconstituent_;
    string sizeconsituent_;
    double regression_p_value_threshold_;
    double distance_coeff_;
    double error_stdev_;
    double error_stdev_isotope_;
    double epsilon_;

    // UI and Tracking
    ProgressWindow* rtw_ = nullptr;
    list<string> tools_used_;
    QMap<QString, double> options_;

    // ========== Private Helper Methods ==========


       /**
     * @brief Finds and retrieves an elemental profile by sample name
     *
     * Searches all groups (sources and target) for a sample with the given name
     * and returns a pointer to its elemental profile. Useful when the group
     * containing the sample is unknown.
     *
     * @param sample_name Name of the sample to find
     * @return Pointer to the elemental profile if found, nullptr otherwise
     *
     * @note Searches all groups sequentially until sample is found
     * @note Returns pointer to internal data - do not delete
     */
    Elemental_Profile* GetElementalProfile(const string& sample_name);

    // --- Element Role Conversion ---

    /**
     * @brief Converts element role enum to string representation
     *
     * Converts the element_information::role enumeration to a human-readable
     * string for serialization, display, or file output.
     *
     * Mappings:
     * - do_not_include → "DoNotInclude"
     * - element → "Element"
     * - isotope → "Isotope"
     * - particle_size → "ParticleSize"
     * - organic_carbon → "OM"
     *
     * @param role Element role enumeration value
     * @return QString representation of the role
     *
     * @note Returns "DoNotInclude" for unrecognized values
     */
    QString Role(const element_information::role& role);

    /**
     * @brief Converts string representation to element role enum
     *
     * Parses a string and returns the corresponding element_information::role
     * enumeration for deserialization or configuration loading.
     *
     * Mappings:
     * - "DoNotInclude" → do_not_include
     * - "Element" → element
     * - "Isotope" → isotope
     * - "ParticleSize" → particle_size
     * - "OM" → organic_carbon
     *
     * @param role_string QString representation of the role
     * @return element_information::role enumeration value
     *
     * @note Returns do_not_include for unrecognized strings
     */
    element_information::role Role(const QString& role_string);

    // --- Parameter Management ---

     /**
     * @brief Retrieves a single parameter value by index
     *
     * @param index Parameter index in the parameters_ vector
     * @return Parameter value at the specified index
     *
     * @note No bounds checking - ensure index is valid
     */
    double GetParameterValue(size_t index) const;

    /**
     * @brief Sets multiple parameter values from a vector
     *
     * Updates all parameters using values from the provided vector. Each parameter
     * update triggers synchronization with the corresponding model component.
     *
     * @param values Vector containing new parameter values
     * @return true if all parameters were successfully updated, false if any update failed
     *
     * @note Updates are applied sequentially; partial updates possible if one fails
     */
    bool SetParameterValue(const CVector& values);

    /**
     * @brief Retrieves all current parameter values as a vector
     *
     * Returns a vector containing all parameter values in the order they appear
     * in the parameters_ vector. The parameter layout depends on estimation_mode_.
     *
     * @return Vector of all parameter values
     */
    CVector GetParameterValue() const;

    // --- Contribution Initialization ---

    /**
     * @brief Initialize source contributions randomly (linear constraint)
     *
     * Randomly initializes contribution values for all sources such that
     * they sum to 1.0 and all values are non-negative. Uses uniform random
     * sampling with rejection until valid contributions are found.
     *
     * @return Always returns true
     */
    bool InitializeContributionsRandomly();

    /**
     * @brief Initialize source contributions randomly (softmax transformation)
     *
     * Randomly initializes contribution values using softmax transformation
     * of normally distributed random variables. This ensures contributions
     * automatically sum to 1.0 and are non-negative without rejection sampling.
     *
     * Softmax transformation: contribution_i = exp(X_i) / sum(exp(X_j))
     * where X ~ N(0,1)
     *
     * @return Always returns true
     */
    bool InitializeContributionsRandomlySoftmax();

    // --- Contribution Accessors ---

    /**
     * @brief Retrieves the contributions from all sources
     *
     * Calculates source contributions where the first (size-2) sources have their
     * contributions stored as parameters, and the last source's contribution is
     * calculated as the remainder to ensure all contributions sum to 1.
     *
     * @return CVector of size (size()-1) containing contribution values for each source
     *
     * @note The last element is computed as (1 - sum of other contributions)
     */
    CVector GetSourceContributions();

    /**
     * @brief Retrieves the current source contribution fractions
     *
     * Returns a vector of contribution fractions from each source. Contributions
     * represent the fraction of the target sample originating from each source,
     * subject to the constraint: Σ f_i = 1.
     *
     * @param include_all If true, returns all n contributions including the
     *                    constrained one. If false, returns only the n-1
     *                    independent contributions used in optimization.
     * @return Vector of contribution fractions, size n or n-1
     *
     * @note All contributions satisfy: 0 ≤ f_i ≤ 1 and Σ f_i = 1
     */
    CVector GetContributionVector(bool include_all = true);

    /**
     * @brief Retrieves the softmax parameters for source contributions
     *
     * Returns the unconstrained softmax parameters x_i that are transformed to
     * contribution fractions via: f_i = exp(x_i) / Σ exp(x_j)
     *
     * @return Vector of softmax parameters, size n (all sources)
     *
     * @note Softmax parameters are unbounded: x_i ∈ (-∞, +∞)
     */
    CVector GetContributionVectorSoftmax();

    /**
     * @brief Sets a single source contribution value
     *
     * Updates the contribution fraction for the specified source and automatically
     * recalculates the last source's contribution to maintain the sum constraint.
     *
     * @param source_index Index of the source to update (0-based)
     * @param contribution_value New contribution fraction (must satisfy 0 ≤ f ≤ 1)
     *
     * @note Last contribution is automatically updated: f_n = 1 - Σ(f_1...f_{n-1})
     */
    void SetContribution(size_t source_index, double contribution_value);

    /**
     * @brief Sets a single softmax parameter value
     *
     * Updates the unconstrained softmax parameter for the specified source.
     * Does NOT automatically update the actual contribution fractions.
     *
     * @param source_index Index of the source to update (0-based)
     * @param softmax_value New softmax parameter (unbounded: x ∈ ℝ)
     *
     * @note This only updates the softmax parameter, not the contribution
     */
    void SetContributionSoftmax(size_t source_index, double softmax_value);

    /**
     * @brief Sets all source contributions from a vector
     *
     * Updates contributions for multiple sources. If the vector has n-1 elements,
     * the last contribution is computed automatically.
     *
     * @param contributions Vector of contribution values
     *
     * @note Validates that all contributions are non-negative
     */
    void SetContribution(const CVector& contributions);

    /**
     * @brief Sets all source contributions using softmax transformation
     *
     * Applies the softmax transformation to convert unconstrained parameters to
     * valid contribution fractions: f_i = exp(x_i) / Σ exp(x_j)
     *
     * @param softmax_params Vector of unconstrained softmax parameters
     *
     * @note This is the correct way to update contributions in softmax mode
     */
    void SetContributionSoftmax(const CVector& softmax_params);

    // --- Distribution Parameter Accessors ---

    /**
     * @brief Retrieves pointer to the μ (mean) parameter for an element distribution
     *
     * Accesses the parameter object representing the mean (μ) of the log-normal
     * distribution for a specific element in a specific source.
     *
     * @param element_index Index of the element in element_order_ (0-based)
     * @param source_index Index of the source in samplesetsorder_ (0-based)
     * @return Pointer to the Parameter object, or nullptr if indices are out of bounds
     *
     * @note For log-normal distributions: actual mean = exp(μ + σ²/2)
     */
    Parameter* GetElementDistributionMuParameter(size_t element_index, size_t source_index);

    /**
     * @brief Retrieves pointer to the σ (std dev) parameter for an element distribution
     *
     * Accesses the parameter object representing the standard deviation (σ) of the
     * log-normal distribution for a specific element in a specific source.
     *
     * @param element_index Index of the element in element_order_ (0-based)
     * @param source_index Index of the source in samplesetsorder_ (0-based)
     * @return Pointer to the Parameter object, or nullptr if indices are out of bounds
     *
     * @note σ represents log-space standard deviation for log-normal distributions
     */
    Parameter* GetElementDistributionSigmaParameter(size_t element_index, size_t source_index);

    /**
     * @brief Retrieves the current value of the μ parameter for an element distribution
     *
     * @param element_index Index of the element in element_order_ (0-based)
     * @param source_index Index of the source in samplesetsorder_ (0-based)
     * @return Current μ value, or 0.0 if parameter cannot be retrieved
     */
    double GetElementDistributionMuValue(size_t element_index, size_t source_index);

    /**
     * @brief Retrieves the current value of the σ parameter for an element distribution
     *
     * @param element_index Index of the element in element_order_ (0-based)
     * @param source_index Index of the source in samplesetsorder_ (0-based)
     * @return Current σ value, or 0.0 if parameter cannot be retrieved
     */
    double GetElementDistributionSigmaValue(size_t element_index, size_t source_index);

    // --- Observation Data ---

    /**
     * @brief Retrieves the observed elemental data for a selected target sample
     *
     * Extracts the elemental concentration values for a specific target sample,
     * returning them in the order specified by element_order_.
     *
     * @param SelectedTargetSample Name of the target sample (empty = use selected_target_sample_)
     * @return CVector containing elemental concentration values
     */
    CVector ObservedDataforSelectedSample(const string& SelectedTargetSample = "");

    /**
     * @brief Retrieves the observed isotopic data for a selected target sample
     *
     * Extracts and converts isotopic ratio values for a specific target sample into
     * absolute concentrations using the delta notation formula:
     * concentration = (δ/1000 + 1) × standard_ratio × base_element_concentration
     *
     * @param SelectedTargetSample Name of the target sample (empty = use selected_target_sample_)
     * @return CVector containing converted isotopic concentration values
     */
    CVector ObservedDataforSelectedSample_Isotope(const string& SelectedTargetSample = "");

    /**
     * @brief Retrieves the observed isotopic data in delta notation
     *
     * Extracts isotopic ratio values for a specific target sample in their original
     * delta notation (‰) format, without conversion to absolute concentrations.
     *
     * @param SelectedTargetSample Name of the target sample (empty = use selected_target_sample_)
     * @return CVector containing isotopic delta values (‰)
     */
    CVector ObservedDataforSelectedSample_Isotope_delta(const string& SelectedTargetSample = "");

    // --- Prediction Methods ---

    /**
     * @brief Predicts target sample elemental concentrations based on source contributions
     *
     * Calculates the predicted elemental composition of the target sample as a linear
     * combination of source profiles weighted by their contributions:
     * C_predicted = Σ (source_mean_i × contribution_i)
     *
     * @param param_mode Parameter mode for obtaining source mean values
     * @return CVector of predicted elemental concentrations
     */
    CVector PredictTarget(parameter_mode param_mode = parameter_mode::direct);

    /**
     * @brief Predicts target sample isotopic compositions based on source contributions
     *
     * Calculates the predicted isotopic composition (as absolute concentrations) of
     * the target sample as a linear combination of source isotopic profiles.
     *
     * @param param_mode Parameter mode for obtaining source mean values
     * @return CVector of predicted isotopic concentrations (absolute values)
     */
    CVector PredictTarget_Isotope(parameter_mode param_mode = parameter_mode::direct);

    /**
     * @brief Predicts target sample isotopic compositions in delta notation
     *
     * Calculates the predicted isotopic composition in delta notation (‰) by
     * first computing absolute concentrations, then converting back to delta values.
     *
     * @param param_mode Parameter mode for obtaining source mean values
     * @return CVector of predicted isotopic delta values (‰)
     */
    CVector PredictTarget_Isotope_delta(parameter_mode param_mode = parameter_mode::based_on_fitted_distribution);

    /**
     * @brief Builds the source mean concentration matrix for chemical elements
     *
     * Constructs a matrix where each entry [i,j] represents the mean concentration
     * of element i in source j. Used in the mixing model equation: C_target = S × f
     *
     * @param param_mode Controls whether to use parametric or empirical means
     * @return Matrix of size (num_elements × num_sources) containing mean concentrations
     *
     * @note Elements are ordered according to element_order_ vector
     * @note Sources are ordered according to samplesetsorder_ vector
     */
    CMatrix BuildSourceMeanMatrix(parameter_mode param_mode = parameter_mode::based_on_fitted_distribution);

    /**
     * @brief Builds the source mean concentration matrix for isotopes
     *
     * Constructs a matrix where each entry [i,j] represents the mean absolute
     * concentration of isotope i in source j. Converts from delta notation to
     * absolute concentrations.
     *
     * Conversion formula: [isotope] = (δ/1000 + 1) × R_standard × [base_element]
     *
     * @param param_mode Controls whether to use parametric or empirical means
     * @return Matrix of size (num_isotopes × num_sources)
     *
     * @note Mixing occurs in concentration space, not delta space
     */
    CMatrix BuildSourceMeanMatrix_Isotopes(parameter_mode param_mode = parameter_mode::based_on_fitted_distribution);

	// --- Likelihood and Prior Calculations ---
    /**
     * @brief Calculates the log prior probability for source contributions
     *
     * Evaluates whether the current source contributions are physically valid by checking
     * if all contributions are non-negative. Acts as a constraint in Bayesian inference.
     *
     * @return -1e10 if any contribution is negative, 0 if all valid
     */
    double LogPriorContributions();

    /**
     * @brief Calculates the log-likelihood of source elemental distributions
     *
     * Computes the total log-likelihood by evaluating how well each source sample's
     * observed elemental concentrations fit their respective estimated distributions.
     *
     * @return Sum of log-probabilities across all elements, all source groups, and all samples
     */
    double LogLikelihoodSourceElementalDistributions();

    /**
     * @brief Calculates the log-likelihood of the model prediction versus measured data
     *
     * Compares the model's predicted elemental concentrations against the observed data
     * for the selected target sample. Computed in log-space assuming log-normally
     * distributed errors.
     *
     * Formula: log(L) = -n*log(σ) - ||log(C_pred) - log(C_obs)||² / (2σ²)
     *
     * @param est_mode Estimation mode determining how predictions are made
     * @return Log-likelihood value (or -1e10 if any predicted concentration is non-positive)
     */
    double LogLikelihoodModelvsMeasured(estimation_mode est_mode = estimation_mode::elemental_profile_and_contribution);

    /**
     * @brief Calculates the log-likelihood of model versus measured isotopic data
     *
     * Compares the model's predicted isotopic delta values against the observed isotopic data
     * for the selected target sample. Operates in linear space assuming normally
     * distributed errors in delta values.
     *
     * Formula: log(L) = -n*log(σ_iso) - ||δ_pred - δ_obs||² / (2σ_iso²)
     *
     * @param est_mode Estimation mode determining how predictions are made
     * @return Log-likelihood value
     */
    double LogLikelihoodModelvsMeasured_Isotope(estimation_mode est_mode = estimation_mode::elemental_profile_and_contribution);

    // --- Optimization Helper Methods ---

    /**
     * @brief Computes the normalized gradient of the log-likelihood function
     *
     * Calculates the gradient vector ∇log(L) using numerical differentiation
     * (finite differences). The gradient points in the direction of steepest
     * ascent for the log-likelihood.
     *
     * Numerical Differentiation:
     *   ∂log(L)/∂θ_i ≈ [log(L(θ + ε·e_i)) - log(L(θ))] / ε
     *
     * @param parameters Parameter vector at which to evaluate the gradient
     * @param est_mode Estimation mode determining which likelihood components to include
     * @return Normalized gradient vector (unit length: ||∇log(L)|| = 1)
     *
     * @note The gradient is normalized by its L2 norm for numerical stability
     */
    CVector Gradient(const CVector& parameters, estimation_mode est_mode);

     /**
     * @brief Calculates the combined residual vector for elemental and isotopic predictions
     *
     * Computes residuals between predicted and observed values for both elemental
     * concentrations (in log-space) and isotopic delta values (in linear space).
     *
     * Combined residual vector structure:
     * [log(C_pred/C_obs) for each element, δ_pred - δ_obs for each isotope]
     *
     * @return CVector of combined residuals
     *
     * @note Returns vector with non-finite values if predictions are invalid
     */
    CVector ResidualVector();

    /**
     * @brief Calculates the combined residual vector using Armadillo vector format
     *
     * Armadillo-based implementation of residual calculation for compatibility with
     * linear algebra operations and optimization routines that use Armadillo types.
     *
     * @return CVector_arma of combined residuals in Armadillo format
     */
    CVector_arma ResidualVector_arma();

    /**
     * @brief Calculates the Jacobian matrix of residuals with respect to contributions (Armadillo)
     *
     * Computes the numerical derivative of the residual vector with respect to the first
     * (n-1) source contributions using finite differences.
     *
     * The Jacobian matrix J has dimensions [(n-1) sources × (elements + isotopes)],
     * where J[i,j] = ∂residual_j / ∂contribution_i
     *
     * @return CMatrix_arma Jacobian matrix in Armadillo format
     *
     * @note Uses adaptive epsilon based on distance from contribution = 0.5
     */
    CMatrix_arma ResidualJacobian_arma();

    /**
     * @brief Calculates the Jacobian matrix of residuals with respect to contributions
     *
     * Computes the numerical derivative of the residual vector with respect to the first
     * (n-1) source contributions using finite differences.
     *
     * @return CMatrix Jacobian matrix
     *
     * @note Uses adaptive epsilon based on distance from contribution = 0.5
     */
    CMatrix ResidualJacobian();

    /**
     * @brief Calculates the Jacobian using softmax parameterization of contributions
     *
     * Computes the numerical derivative of residuals with respect to unconstrained
     * softmax parameters. Includes all n sources since softmax transformation ensures
     * contributions sum to 1 automatically.
     *
     * @return CMatrix Jacobian matrix (n sources × m observations)
     *
     * @note Uses sign-dependent epsilon for better numerical behavior
     */
    CMatrix ResidualJacobian_softmax();

    /**
     * @brief Performs one iteration of the Levenberg-Marquardt optimization algorithm
     *
     * Computes the parameter update step for source contributions using the
     * Levenberg-Marquardt algorithm, which interpolates between Gauss-Newton
     * and gradient descent methods.
     *
     * The algorithm solves: (J^T J + λ diag(J^T J)) dx = -J^T r
     *
     * @param lambda Damping parameter controlling trade-off (typical: 0.001 to 1000)
     * @return CVector parameter update vector dx of size (n-1)
     *
     * @note Uses standard contribution parameterization (last source implicit)
     */
    CVector OneStepLevenberg_Marquardt(double lambda);

    /**
     * @brief Performs one iteration of Levenberg-Marquardt using softmax parameterization
     *
     * Computes the parameter update step for softmax-parameterized source contributions.
     *
     * @param lambda Damping parameter
     * @return CVector parameter update vector dx of size n
     *
     * @note Uses softmax parameterization where all n sources are independent
     */
    CVector OneStepLevenberg_Marquardt_softmax(double lambda);

     /**
     * @brief Retrieves the ordering of source groups
     *
     * @return Vector of source group names in parameter order
     */
    vector<string> GetSourceOrder() const;

    /**
     * @brief Retrieves the ordering of sample sets
     *
     * @return Vector of sample set names
     */
    vector<string> SamplesetsOrder();

    /**
     * @brief Retrieves the ordering of all constituents
     *
     * @return Vector of all constituent names
     */
    vector<string> ConstituentOrder();

    /**
     * @brief Retrieves the ordering of chemical elements
     *
     * @return Vector of element names in analysis order
     */
    vector<string> ElementOrder();

    /**
     * @brief Retrieves the ordering of isotopes
     *
     * @return Vector of isotope names in analysis order
     */
    vector<string> IsotopeOrder();

    /**
     * @brief Retrieves the ordering of size and OM constituents
     *
     * @return Vector of size/OM constituent names
     */
    vector<string> SizeOMOrder();

    // --- Statistical Methods ---

    /**
     * @brief Counts the total number of source samples across all source groups
     *
     * Sums the number of samples in all source groups, excluding the target group.
     *
     * @return Total number of source samples
     *
     * @note Target group samples are excluded
     */
    int TotalNumberofSourceSamples() const;

     /**
     * @brief Computes grand mean concentration for an element across all sources
     *
     * Calculates the overall mean concentration weighted by sample sizes across
     * all source groups (excluding target).
     *
     * Formula: μ_grand = Σ(n_i × μ_i) / Σ(n_i)
     *
     * @param element Name of the element
     * @param use_log If true, compute using log-space means
     * @return Weighted grand mean concentration
     *
     * @note Target group is excluded
     */
    double GrandMean(const string& element, bool use_log);

    /**
     * @brief Combines all source samples into a single profile set
     *
     * Pools samples from all source groups into one unified Elemental_Profile_Set.
     *
     * @return Elemental_Profile_Set containing all source samples combined
     *
     * @note Target group is excluded
     */
    Elemental_Profile_Set LumpAllProfileSets();

    // --- Box-Cox Transformation ---

    /**
     * @brief Computes optimal Box-Cox transformation parameters for all elements
     *
     * Determines the optimal λ (lambda) parameter for each element's Box-Cox
     * transformation by maximizing the log-likelihood of achieving normality.
     *
     * Box-Cox transformation: y = (x^λ - 1) / λ  (or ln(x) if λ = 0)
     *
     * Special cases:
     * - λ = 1: No transformation (linear)
     * - λ = 0.5: Square root transformation
     * - λ = 0: Log transformation
     * - λ = -1: Reciprocal transformation
     *
     * @return CMBVector of optimal λ parameters, labeled with element names
     *
     * @note Search range: λ ∈ [-5, 5]
     * @note Optimization iterations: 10
     */
    CMBVector OptimalBoxCoxParameters();

    // --- Bootstrap Helpers ---

    /**
     * @brief Creates dataset with randomly excluded source samples for validation
     *
     * Generates a new dataset with a specified percentage of source samples
     * randomly removed. Useful for bootstrap validation and uncertainty analysis.
     *
     * @param percentage Percentage of samples to exclude (0-100)
     * @return New SourceSinkData object with reduced source samples
     *
     * @note Target group samples are preserved
     * @note Element distributions are recalculated
     */
    SourceSinkData RandomlyEliminateSourceSamples(const double& percentage);

    /**
     * @brief Retrieves names of all samples across all source groups
     *
     * Collects sample names from all source groups (excluding target) into a
     * single flat list.
     *
     * @return Vector of all source sample names
     *
     * @note Target group samples are excluded
     */
    vector<string> AllSourceSampleNames() const;

    /**
     * @brief Randomly selects a subset of source samples
     *
     * Performs Bernoulli sampling on all source samples, where each sample is
     * independently included with the specified probability.
     *
     * @param percentage Probability of selecting each sample (0.0 to 1.0)
     * @return Vector of randomly selected sample names
     *
     * @note Expected number of samples: N × percentage
     * @note Each sample is selected independently
     */
    vector<string> RandomlypickSamples(const double& percentage) const;

    /**
     * @brief Creates a new dataset with a source sample designated as the target
     *
     * Generates a modified dataset where a specified source sample becomes the
     * new target sample. Useful for leave-one-out validation.
     *
     * @param source_sample_name Name of the source sample to use as new target
     * @return New SourceSinkData object with specified sample as target
     *
     * @note Source sample is copied to target group (not moved)
     */
    SourceSinkData ReplaceSourceAsTarget(const string& source_sample_name) const;

    // --- DFA Helper Methods ---

    /**
     * @brief Computes pooled within-group covariance matrix
     *
     * Calculates the weighted average of covariance matrices within each source
     * group. Represents the "error" or within-group scatter in discriminant analysis.
     *
     * Formula: S_W = Σ[(n_i - 1) × Σ_i] / Σ(n_i - 1)
     *
     * @return Within-group covariance matrix (num_elements × num_elements)
     *
     * @note Target group excluded
     * @note Weighted by degrees of freedom (n_i - 1)
     */
    CMatrix WithinGroupCovarianceMatrix();

    /**
     * @brief Computes between-group covariance matrix
     *
     * Calculates covariance matrix based on deviations of group means from the
     * overall mean. Represents the "signal" or between-group scatter.
     *
     * Formula: S_B = Σ[n_i × (μ_i - μ_overall)(μ_i - μ_overall)ᵀ] / N
     *
     * @return Between-group covariance matrix (num_elements × num_elements)
     *
     * @note Target group excluded
     */
    CMatrix BetweenGroupCovarianceMatrix();

    /**
     * @brief Computes total scatter matrix
     *
     * Calculates the overall covariance matrix treating all source samples as a
     * single population. Represents total variability.
     *
     * Formula: S_T = Σ Σ[(x_ij - μ_overall)(x_ij - μ_overall)ᵀ] / N
     *
     * Relationship: S_T = S_W + S_B
     *
     * @return Total scatter matrix (num_elements × num_elements)
     *
     * @note Target group excluded
     */
    CMatrix TotalScatterMatrix();

    /**
     * @brief Computes Wilks' Lambda statistic for multivariate group separation
     *
     * Calculates Wilks' Lambda, a multivariate test statistic measuring the ratio
     * of within-group to total variance.
     *
     * Formula: Λ = |S_W| / |S_T| = |S_W| / |S_W + S_B|
     *
     * Interpretation:
     * - Λ = 1: No group separation
     * - Λ → 0: Perfect separation
     *
     * @return Wilks' Lambda statistic (0 to 1)
     *
     * @note Uses absolute values of determinants for numerical stability
     */
    double WilksLambda();

    /**
     * @brief Computes p-value for discriminant function analysis
     *
     * Tests the null hypothesis that all source groups have the same mean
     * elemental composition using Wilks' Lambda transformed to chi-squared.
     *
     * Chi-squared transformation:
     * χ² = -[N - 1 - (p + k)/2] × ln(Λ)
     *
     * @return P-value for test of group equality
     *
     * @note P-value < 0.05 indicates significant differences
     */
    double DFA_P_Value();

    /**
     * @brief Projects all groups onto the discriminant function axis
     *
     * Computes discriminant scores for all samples in all groups by projecting
     * elemental profiles onto the primary discriminant eigenvector.
     *
     * @return CMBVectorSet with discriminant scores for each group
     */
    CMBVectorSet DFA_Projected();

    /**
     * @brief Projects two specific groups onto the discriminant function axis
     *
     * @param source1 Name of first source group
     * @param source2 Name of second source group
     * @return CMBVectorSet with discriminant scores
     *
     * @note Current implementation projects all groups
     */
    CMBVectorSet DFA_Projected(const string& source1, const string& source2);

    /**
     * @brief Projects samples from original dataset onto discriminant function
     *
     * Uses discriminant function computed from current dataset to project samples
     * from the original dataset. Useful for validating on held-out data.
     *
     * @param source1 Name of source used in discriminant function
     * @param original Pointer to original dataset
     * @return CMBVectorSet with discriminant scores for original samples
     *
     * @note Target group from original dataset is excluded
     */
    CMBVectorSet DFA_Projected(const string& source1, SourceSinkData* original);

    /**
     * @brief Computes the primary discriminant function eigenvector
     *
     * Calculates the eigenvector corresponding to the largest eigenvalue of
     * S_W⁻¹ × S_B, which defines the discriminant function axis.
     *
     * @return CMBVector containing discriminant function coefficients
     *
     * @note Returns empty vector if S_W is singular
     * @note Uses largest eigenvalue by absolute value
     */
    CMBVector DFA_eigvector();

    /**
     * @brief Computes discriminant weight vector for two-group comparison
     *
     * Calculates the linear discriminant weights for separating two specific
     * source groups using Fisher's linear discriminant analysis.
     *
     * Formula: w = (μ₂ - μ₁) / (Σ₁ + Σ₂)
     *
     * @param source1 Name of first source group
     * @param source2 Name of second source group
     * @return Normalized CMBVector of discriminant weights (unit length)
     */
    CMBVector DFA_weight_vector(const string& source1, const string& source2);

    /**
     * @brief Projects samples onto a discriminant function axis
     *
     * Transforms elemental profiles from a source group by computing their
     * projections (dot products) with a discriminant eigenvector.
     *
     * Projection formula: score_i = Σ(x_ij × w_j)
     *
     * @param eigenvector Discriminant function coefficients
     * @param source_group Name of the source group to project
     * @return CMBVector containing discriminant scores, labeled with sample names
     */
    CMBVector DFATransformed(const CMBVector& eigenvector, const string& source_group);

    /**
     * @brief Collects all source samples except those from a specified source group
     *
     * Creates a combined profile set containing samples from all source groups
     * except the specified one and the target group. Useful for "one vs rest" comparisons.
     *
     * @param excluded_source Name of the source group to exclude
     * @return Elemental_Profile_Set containing all samples from other sources
     *
     * @note Target group is always excluded
     */
    Elemental_Profile_Set TheRest(const string& excluded_source);

    // --- Mean Calculation ---

    /**
     * @brief Computes mean elemental concentrations for a specific group
     *
     * Calculates the mean concentration for each element within a specified
     * source or target group.
     *
     * @param group_name Name of the group (source or target)
     * @return CMBVector of mean concentrations, labeled with element names
     *
     * @note Returns empty vector if group doesn't exist
     */
    CMBVector MeanElementalContent(const string& group_name);

    /**
     * @brief Computes weighted mean elemental concentrations across all sources
     *
     * Calculates the overall mean concentration for each element, weighted by
     * the number of samples in each source group.
     *
     * Formula: μ_overall = Σ(n_i × μ_i) / Σ(n_i)
     *
     * @return CMBVector of weighted mean concentrations, labeled with element names
     *
     * @note Target group excluded
     * @note Weighting ensures larger groups have proportional influence
     */
    CMBVector MeanElementalContent();

    
};

#endif // SOURCESINKDATA_H
