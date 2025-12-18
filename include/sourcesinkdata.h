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

enum class transformation {linear, softmax};

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


/*struct DFA_result_vector
{
    CMBVector eigen_vector;
    CMBVector significance_vector;
    CMBVector significance_vector_2;
    double S_value;
};*/

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

enum class estimation_mode {only_contributions, elemental_profile_and_contribution, source_elemental_profiles_based_on_source_data};

enum class negative_reporting {list, complete};

class SourceSinkData: public map<string, Elemental_Profile_Set>
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
     * @brief Count the number of elements in the dataset
     *
     * Counts elements based on their inclusion status and role. Can count either:
     * - All elements (if exclude_elements = false)
     * - Only elements marked for analysis, excluding OM, particle size, and do_not_include roles (if exclude_elements = true)
     *
     * @param exclude_elements If true, only count elements marked for analysis with valid roles
     * @return Number of elements meeting the criteria
     */
    int CountElements(bool exclude_elements) const;

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

    Elemental_Profile *GetElementalProfile(const string sample_name); //Get Elemental Profile for a particular sample
    
    /**
     * @brief Populate element distributions from all groups
     *
     * Builds concentration distributions for each element by collecting
     * all concentration values across all groups (sources and target).
     * First updates distributions within each group, then aggregates
     * them into the overall element_distributions_ map.
     *
     * This should be called after loading data or modifying profiles
     * to ensure distributions are up-to-date for statistical analysis.
     */
    void PopulateElementDistributions();

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
     * This should be called after PopulateElementDistributions() to ensure
     * all distributions are properly fitted for statistical analysis.
     */
    void AssignAllDistributions();


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

    static QString Role(const element_information::role &rl);
    static element_information::role Role(const QString &rl);
    element_information* GetElementInformation(const string &element_name)
    {
        if (element_information_.count(element_name))
            return  &element_information_.at(element_name);
        else
            return nullptr;
    }
    map<string,element_information>* GetElementInformation()
    {
        return &element_information_;
    }
    ConcentrationSet* GetElementDistribution(const string &element_name)
    {
        if (element_distributions_.count(element_name))
            return  &element_distributions_.at(element_name);
        else
            return nullptr;
    }
    ConcentrationSet* GetElementDistribution(const string &element_name, const string &sample_group)
    {
        if (!GetSampleSet(sample_group))
        {
            cout<<"Sample Group '" + sample_group +"' does not exist!"<<std::endl;
            return nullptr;

        }
        if (!GetSampleSet(sample_group)->GetElementDistribution(element_name))
        {
            cout<<"Element '" + element_name +"' does not exist!"<<std::endl;
            return nullptr;
        }

        return  GetSampleSet(sample_group)->GetElementDistribution(element_name);

    }
    


    bool Execute(const string &command, const map<string,string> &arguments);
    /**
     * @brief Get the output directory path
     *
     * Returns the path where analysis results and output files will be saved.
     *
     * @return Current output path
     */
    string GetOutputPath() const;

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

    vector<Parameter> &Parameters() {return parameters_;}
    size_t ParametersCount() {return parameters_.size(); }
    size_t ObservationsCount() {return observations_.size(); }
    Parameter *parameter(size_t i)
    {
        if (i>=0 && i<parameters_.size())
            return &parameters_[i];
        else
            return nullptr;
    }

    const Parameter* parameter(size_t i) const
    {
        if (i >= 0 && i < parameters_.size())
            return &parameters_[i];
        else
            return nullptr;
    }

    Observation *observation(size_t i)
    {
        if (i>=0 && i<observations_.size())
            return &observations_[i];
        else
            return nullptr;
    }
    
    bool SetTargetGroup(const string &targroup)
    {
	    target_group_ = targroup;
        return true; 
    }
    string GetTargetGroup() {return target_group_;}
    
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
     * For each mode, appropriate prior distributions and parameter bounds are set.
     * Observations are created from the target sample's elemental concentrations.
     *
     * This must be called before running MCMC to set up the optimization problem.
     *
     * @param targetsamplename Name of the target sample to unmix
     * @param est_mode Estimation mode controlling which parameters to optimize
     * @return true if successful, false if data not loaded
     */
    bool InitializeParametersAndObservations(const string& targetsamplename, estimation_mode est_mode = estimation_mode::elemental_profile_and_contribution);
        
    /**
     * @brief Initialize source contributions randomly (linear constraint)
     *
     * Randomly initializes contribution values for all sources such that
     * they sum to 1.0 and all values are non-negative. Uses uniform random
     * sampling with rejection until valid contributions are found.
     *
     * Linear transformation ensures: sum(contributions) = 1.0, all >= 0
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


    bool SetParameterValue(unsigned int i, double value); //set the parameter values for estimation
    bool SetParameterValue(const CVector &value); //set the parameter values for estimation
    CVector GetParameterValue(); // return the vector of all the parameter values
    double GetParameterValue(unsigned int i); // return the value of parameter i
    Elemental_Profile Sample(const string &samplename) const; //extract a single sample
    CVector Gradient(const CVector &value, const estimation_mode estmode); //calculates the gradient of likelihood function
    CVector GradientUpdate(const estimation_mode estmode = estimation_mode::elemental_profile_and_contribution); //Improve the estimate by one step using the gradient descent method
    /**
     * @brief Predicts target sample elemental concentrations based on source contributions
     *
     * Calculates the predicted elemental composition of the target sample as a linear
     * combination of source profiles weighted by their contributions:
     *
     * C_predicted = Σ (source_mean_i × contribution_i)
     *
     * This is the core mixing model prediction, where each element's concentration
     * is the sum of contributions from all sources.
     *
     * The function also updates the predicted values stored in observation objects
     * for later comparison with measured values.
     *
     * @param param_mode Parameter mode for obtaining source mean values:
     *        - parameter_mode::direct: Use current parameter values directly
     *        - parameter_mode::based_on_fitted_distribution: Use means from fitted distributions
     *
     * @return CVector Predicted elemental concentrations in the order specified by
     *         element_order_. Vector size equals the number of elements.
     */
    CVector PredictTarget(parameter_mode param_mode = parameter_mode::direct);

    /**
     * @brief Predicts target sample isotopic compositions based on source contributions
     *
     * Calculates the predicted isotopic composition (as absolute concentrations, not
     * delta values) of the target sample as a linear combination of source isotopic
     * profiles weighted by their contributions:
     *
     * C_isotope_predicted = Σ (source_isotope_mean_i × contribution_i)
     *
     * Unlike PredictTarget_Isotope_delta(), this returns absolute isotopic concentrations
     * rather than delta notation values.
     *
     * @param param_mode Parameter mode for obtaining source mean values:
     *        - parameter_mode::direct: Use current parameter values directly
     *        - parameter_mode::based_on_fitted_distribution: Use means from fitted distributions
     *
     * @return CVector Predicted isotopic concentrations (absolute values) in the order
     *         specified by isotope_order_. Vector size equals the number of isotopes.
     */
    CVector PredictTarget_Isotope(parameter_mode param_mode = parameter_mode::direct);
    CVector PredictTarget_Isotope_delta(parameter_mode param_mode= parameter_mode::based_on_fitted_distribution);
    CMatrix SourceMeanMatrix(parameter_mode param_mode = parameter_mode::based_on_fitted_distribution);
    CMatrix SourceMeanMatrix_Isotopes(parameter_mode param_mode = parameter_mode::based_on_fitted_distribution);
    CVector GetContributionVector(bool full=true);
    CVector GetContributionVectorSoftmax();
    void SetContribution(int i, double value);
    void SetContributionSoftmax(int i, double value);
    void SetContribution(const CVector &X);
    void SetContributionSoftmax(const CVector &X); //set source contribution softmax transforations
    
    /**
     * @brief Retrieves the observed elemental data for a selected target sample
     *
     * Extracts the elemental concentration values for a specific target sample,
     * returning them in the order specified by element_order_. If no sample name
     * is provided, uses the internally stored selected_target_sample_.
     *
     * @param SelectedTargetSample The name of the target sample to retrieve data for.
     *        If empty string (""), the function uses the internally stored
     *        selected_target_sample_ member variable.
     *
     * @return CVector A vector containing elemental concentration values for the selected
     *         sample, ordered according to element_order_. The vector size equals the
     *         number of elements in element_order_.
     */
    
     /**
      * @brief Retrieves the observed elemental data for a selected target sample
      *
      * Extracts the elemental concentration values for a specific target sample,
      * returning them in the order specified by element_order_. If no sample name
      * is provided, uses the internally stored selected_target_sample_.
      *
      * @param SelectedTargetSample The name of the target sample to retrieve data for.
      *        If empty string (""), the function uses the internally stored
      *        selected_target_sample_ member variable.
      *
      * @return CVector A vector containing elemental concentration values for the selected
      *         sample, ordered according to element_order_. The vector size equals the
      *         number of elements in element_order_.
      */
    CVector ObservedDataforSelectedSample(const string& SelectedTargetSample = "");

    /**
     * @brief Retrieves the observed isotopic data for a selected target sample
     *
     * Extracts and converts isotopic ratio values for a specific target sample into
     * absolute concentrations. The conversion uses the delta notation formula:
     * concentration = (δ/1000 + 1) × standard_ratio × base_element_concentration
     *
     * For each isotope, this function:
     * 1. Identifies the corresponding base element
     * 2. Retrieves the isotope ratio (in delta notation, ‰)
     * 3. Converts to absolute concentration using the standard ratio and base element concentration
     *
     * If no sample name is provided, uses the internally stored selected_target_sample_.
     *
     * @param SelectedTargetSample The name of the target sample to retrieve data for.
     *        If empty string (""), the function uses the internally stored
     *        selected_target_sample_ member variable.
     *
     * @return CVector A vector containing converted isotopic concentration values for the
     *         selected sample, ordered according to isotope_order_. The vector size equals
     *         the number of isotopes in isotope_order_.
     */
    CVector ObservedDataforSelectedSample_Isotope(const string& SelectedTargetSample = "");

    /**
     * @brief Retrieves the observed isotopic data in delta notation for a selected target sample
     *
     * Extracts isotopic ratio values for a specific target sample in their original
     * delta notation (‰) format, without conversion to absolute concentrations.
     * This is the raw delta value representation as stored in the sample data.
     *
     * Unlike ObservedDataforSelectedSample_Isotope(), this function returns the
     * isotope ratios directly as delta values rather than converting them to
     * absolute concentrations using standard ratios and base element concentrations.
     *
     * If no sample name is provided, uses the internally stored selected_target_sample_.
     *
     * @param SelectedTargetSample The name of the target sample to retrieve data for.
     *        If empty string (""), the function uses the internally stored
     *        selected_target_sample_ member variable.
     *
     * @return CVector A vector containing isotopic delta values (‰) for the selected
     *         sample, ordered according to isotope_order_. The vector size equals the
     *         number of isotopes in isotope_order_.
     */
    CVector ObservedDataforSelectedSample_Isotope_delta(const string& SelectedTargetSample = "");

    /**
     * @brief Returns the objective function value for optimization algorithms
     *
     * Computes the negative log-likelihood based on the current parameter values
     * and estimation mode. Used by optimization algorithms (Levenberg-Marquardt,
     * gradient descent) where minimizing -log(L) is equivalent to maximizing
     * likelihood L.
     *
     * @return Negative log-likelihood value (lower values indicate better fit)
     *
     * @note Uses parameter_estimation_mode_ to determine which likelihood
     *       components to include
     *
     * @see LogLikelihood() for likelihood calculation details
     */
    double GetObjectiveFunctionValue();

    /**
     * @brief Calculates the total log-likelihood for Bayesian source apportionment
     *
     * Computes the posterior log-likelihood by summing multiple components based
     * on the estimation mode. Components include source data likelihood, observation
     * likelihood (elements and isotopes), and contribution priors.
     *
     * Likelihood Components by Estimation Mode:
     * - only_contributions: log P(C_obs|f) + log P(δ_obs|f) + log P(f)
     * - elemental_profile_and_contribution: log P(Y|μ,σ) + log P(C_obs|f,μ,σ) + log P(δ_obs|f,μ,σ) + log P(f)
     * - source_elemental_profiles_based_on_source_data: log P(Y|μ,σ) + log P(f)
     *
     * @param est_mode Estimation mode determining which likelihood terms to include
     *
     * @return Total log-likelihood (higher values indicate better model fit)
     *
     * @see LogLikelihoodSourceElementalDistributions() for source data term
     * @see LogLikelihoodModelVsObserved() for element observation term
     * @see LogLikelihoodModelVsObserved_Isotope() for isotope observation term
     * @see LogPriorContributions() for contribution prior term
     */
    double LogLikelihood(estimation_mode est_mode = estimation_mode::elemental_profile_and_contribution);

    vector<string> ElementsToBeUsedInCMB();
    vector<string> IsotopesToBeUsedInCMB();
    vector<string> SourceGroupNames();
    bool SetSelectedTargetSample(const string &sample_name);
    string SelectedTargetSample();
    vector<string> GetSourceOrder() {return samplesetsorder_;}
    vector<string> SamplesetsOrder() {return samplesetsorder_;}
    vector<string> ConstituentOrder() {return constituent_order_;}
    vector<string> ElementOrder() {return element_order_;}
    vector<string> IsotopeOrder() {return isotope_order_;}
    vector<string> SizeOMOrder() {return size_om_order_;}
    ResultItem GetContribution();
    ResultItem GetPredictedElementalProfile(parameter_mode param_mode = parameter_mode::based_on_fitted_distribution);
    CVector GetPredictedValues(); // copied the predicted constituents and isotopes from observations into a vector
    ResultItem GetPredictedElementalProfile_Isotope(parameter_mode param_mode = parameter_mode::based_on_fitted_distribution);
    ResultItem GetObservedElementalProfile();
    ResultItem GetObservedElementalProfile_Isotope();
    ResultItem GetObservedvsModeledElementalProfile(parameter_mode param_mode = parameter_mode::based_on_fitted_distribution);
    ResultItem GetObservedvsModeledElementalProfile_Isotope(parameter_mode param_mode = parameter_mode::based_on_fitted_distribution);
    vector<ResultItem> GetMLRResults();
    ResultItem GetCalculatedElementMeans();
    vector<ResultItem> GetSourceProfiles();
    ResultItem GetCalculatedElementSigma();
    ResultItem GetCalculatedElementMu();
    ResultItem GetEstimatedElementMu();
    ResultItem GetEstimatedElementMean();
    ResultItem GetEstimatedElementSigma();
    /**
     * @brief Calculates the combined residual vector for elemental and isotopic predictions
     *
     * Computes residuals between predicted and observed values for both elemental
     * concentrations and isotopic delta values. Elemental residuals are calculated in
     * log-space to account for multiplicative errors, while isotopic residuals are
     * calculated in linear space (delta notation).
     *
     * The combined residual vector has the structure:
     * [log(C_pred/C_obs) for each element, δ_pred - δ_obs for each isotope]
     *
     * This residual vector is used in optimization and uncertainty quantification.
     *
     * @return CVector Combined residual vector with elemental log-residuals followed by
     *         isotopic linear residuals. Returns a vector with non-finite values if
     *         predictions are invalid (e.g., non-positive elemental concentrations).
     *
     * @note Logs debug warnings if non-finite values are detected in predictions or residuals
     */
    CVector ResidualVector();

    /**
     * @brief Calculates the combined residual vector using Armadillo vector format
     *
     * Armadillo-based implementation of residual calculation for compatibility with
     * linear algebra operations and optimization routines that use Armadillo types.
     * Functionally equivalent to ResidualVector() but returns CVector_arma type.
     *
     * Computes residuals between predicted and observed values for both elemental
     * concentrations (in log-space) and isotopic delta values (in linear space).
     *
     * @return CVector_arma Combined residual vector in Armadillo format with elemental
     *         log-residuals followed by isotopic linear residuals.
     *
     * @see ResidualVector()
     */
    CVector_arma ResidualVector_arma();
    /**
     * @brief Calculates the Jacobian matrix of residuals with respect to source contributions (Armadillo)
     *
     * Computes the numerical derivative of the residual vector with respect to the first
     * (n-1) source contributions using finite differences. The last source contribution
     * is implicit (1 - sum of others), so it doesn't appear as a parameter.
     *
     * The Jacobian matrix J has dimensions [(n-1) sources × (elements + isotopes)],
     * where J[i,j] = ∂residual_j / ∂contribution_i
     *
     * Uses adaptive epsilon based on distance from contribution = 0.5 to improve
     * numerical stability near boundary values (0 or 1).
     *
     * @return CMatrix_arma Jacobian matrix where each column corresponds to a source
     *         contribution parameter and each row to a residual component (elemental
     *         or isotopic). Armadillo format for compatibility with linear algebra operations.
     *
     * @note This version uses setcol() and epsilon = (0.5 - contribution) * 1e-6
     */
    CMatrix_arma ResidualJacobian_arma();

    /**
     * @brief Calculates the Jacobian matrix of residuals with respect to source contributions
     *
     * Computes the numerical derivative of the residual vector with respect to the first
     * (n-1) source contributions using finite differences. The last source contribution
     * is implicit (1 - sum of others), so it doesn't appear as a parameter.
     *
     * The Jacobian matrix J has dimensions [(n-1) sources × (elements + isotopes)],
     * where J[i,j] = ∂residual_j / ∂contribution_i
     *
     * Uses adaptive epsilon based on distance from contribution = 0.5 to improve
     * numerical stability. Larger epsilon (1e-3) compared to _arma version.
     *
     * @return CMatrix Jacobian matrix where each row corresponds to a source contribution
     *         parameter and each column to a residual component (elemental or isotopic).
     *
     * @note This version uses setrow() and epsilon = (0.5 - contribution) * 1e-3
     */
    CMatrix ResidualJacobian();

    /**
     * @brief Calculates the Jacobian using softmax parameterization of contributions
     *
     * Computes the numerical derivative of residuals with respect to unconstrained
     * softmax parameters. Unlike the standard Jacobian, this includes all n sources
     * as the softmax transformation ensures contributions sum to 1 automatically.
     *
     * The softmax parameterization: contribution_i = exp(x_i) / Σexp(x_j)
     * allows optimization over unconstrained parameters x_i ∈ (-∞, +∞).
     *
     * The Jacobian matrix J has dimensions [n sources × (elements + isotopes)],
     * where J[i,j] = ∂residual_j / ∂x_i (softmax parameter)
     *
     * Uses sign-dependent epsilon to handle positive and negative softmax parameters.
     *
     * @return CMatrix Jacobian matrix where each row corresponds to a softmax parameter
     *         and each column to a residual component. Dimension is (n × m) rather than
     *         ((n-1) × m) since all parameters are independent in softmax space.
     *
     * @note Epsilon = -sign(x_i) * 1e-3 provides better numerical behavior across
     *       the full range of softmax parameters
     */
    CMatrix ResidualJacobian_softmax();
    /**
     * @brief Performs one iteration of the Levenberg-Marquardt optimization algorithm
     *
     * Computes the parameter update step for source contributions using the
     * Levenberg-Marquardt (LM) algorithm, which interpolates between Gauss-Newton
     * and gradient descent methods.
     *
     * The algorithm solves: (J^T J + λ diag(J^T J)) dx = -J^T r
     * where J is the Jacobian, r is the residual vector, and λ controls the step size.
     *
     * - Small λ → Gauss-Newton (fast convergence near minimum)
     * - Large λ → Gradient descent (stable far from minimum)
     *
     * If the normal equations matrix is near-singular (det < 1e-6), additional
     * regularization is applied to ensure numerical stability.
     *
     * @param lambda Damping parameter controlling the trade-off between Gauss-Newton
     *        and gradient descent. Typical values: 0.001 to 1000.
     *        Larger values produce smaller, more conservative steps.
     *
     * @return CVector Parameter update vector dx of size (n-1) where n is the number
     *         of sources. Apply as: contribution[i] += dx[i]
     *
     * @note Uses standard contribution parameterization where the last source
     *       contribution is implicit (1 - sum of others)
     */
    CVector OneStepLevenberg_Marquardt(double lambda);

    /**
     * @brief Performs one iteration of Levenberg-Marquardt using softmax parameterization
     *
     * Computes the parameter update step for softmax-parameterized source contributions
     * using the Levenberg-Marquardt algorithm. The softmax parameterization allows
     * unconstrained optimization while automatically ensuring contributions sum to 1.
     *
     * The algorithm solves: (J^T J + λ diag(J^T J)) dx = -J^T r
     * where J is the Jacobian with respect to softmax parameters.
     *
     * Softmax parameterization: contribution_i = exp(x_i) / Σexp(x_j)
     * This allows x_i ∈ (-∞, +∞) while maintaining valid contributions.
     *
     * @param lambda Damping parameter controlling the trade-off between Gauss-Newton
     *        and gradient descent. Typical values: 0.001 to 1000.
     *        Larger values produce smaller, more conservative steps.
     *
     * @return CVector Parameter update vector dx of size n where n is the number
     *         of sources. Apply as: softmax_param[i] += dx[i], then recompute
     *         contributions via softmax transformation.
     *
     * @note Uses softmax parameterization where all n sources are independent parameters
     */
    CVector OneStepLevenberg_Marquardt_softmax(double lambda);
    /**
 * @brief Solves for optimal source contributions using the Levenberg-Marquardt algorithm
 *
 * Iteratively optimizes source contributions to minimize the residual between predicted
 * and observed elemental/isotopic compositions. The algorithm adaptively adjusts the
 * damping parameter (lambda) based on convergence behavior:
 * - Decreases lambda when error reduces significantly (faster convergence)
 * - Increases lambda when error increases (more stable steps)
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
 * @return bool Currently always returns false (legacy). Consider checking convergence
 *         status: true if converged within tolerance, false if max iterations reached.
 *
 * @note Updates internal state with optimized contributions. If rtw_ (real-time widget)
 *       is set, displays convergence progress graphically.
 */
    bool SolveLevenberg_Marquardt(transformation trans = transformation::linear);

    void SetProgressWindow(ProgressWindow *_rtw) {rtw_ = _rtw;}
    void SetParameterEstimationMode(estimation_mode est_mode) {parameter_estimation_mode = est_mode;}
    estimation_mode ParameterEstimationMode() {return parameter_estimation_mode;}
    bool WriteToFile(QFile *fil);
    bool ReadFromFile(QFile *fil);
    bool WriteElementInformationToFile(QFile *fil);
    bool WriteDataToFile(QFile *file);
    QJsonObject ElementInformationToJsonObject();
    QJsonArray ToolsUsedToJsonObject();
    QJsonObject OptionsToJsonObject();
    void AddtoToolsUsed(const string &tool);
    bool ReadToolsUsedFromJsonObject(const QJsonArray &jsonobject);
    QJsonObject ElementDataToJsonObject();
    bool ReadElementInformationfromJsonObject(const QJsonObject &jsonobject);
    bool ReadElementDatafromJsonObject(const QJsonObject &jsonobject);
    bool ReadOptionsfromJsonObject(const QJsonObject &jsonobject);
    bool Perform_Regression_vs_om_size(const string &om, const string &d, regression_form form=regression_form::linear, const double &p_value_threshold=0.05);
    DFA_result DiscriminantFunctionAnalysis(const string &source1);
    DFA_result DiscriminantFunctionAnalysis();
    DFA_result DiscriminantFunctionAnalysis(const string &source1, const string &source2);

    vector<CMBVector> StepwiseDiscriminantFunctionAnalysis();
    vector<CMBVector> StepwiseDiscriminantFunctionAnalysis(const string &source1, const string &source2);
    vector<CMBVector> StepwiseDiscriminantFunctionAnalysis(const string &source1);
    int TotalNumberofSourceSamples() const;
    CMBVector DFATransformed(const CMBVector &eigenvector, const string &source);
    Elemental_Profile_Set TheRest(const string &source);
    CMBVector BracketTest(const string &target_sample, bool correct_for_OM_n_Size);
    CMBMatrix BracketTest(bool correct_for_OM_n_Size, bool exclude_elements, bool exclude_samples); //Performs bracket test for all target samples
    SourceSinkData BoxCoxTransformed(bool calculateeigenvectorforallgroups=false);
    map<string,ConcentrationSet> ExtractConcentrationSet();
    CMBVector OptimalBoxCoxParameters();
    Elemental_Profile DifferentiationPower(const string &source1, const string &source2, bool log);
    Elemental_Profile DifferentiationPower_Percentage(const string &source1, const string &source2);
    Elemental_Profile t_TestPValue(const string &source1, const string &source2, bool log);
    Elemental_Profile_Set DifferentiationPower(bool log, bool include_target);
    Elemental_Profile_Set DifferentiationPower_Percentage(bool include_target);
    Elemental_Profile_Set DifferentiationPower_P_value(bool include_target);
    vector<string> NegativeValueCheck();
    double GrandMean(const string &element, bool logtransformed);
    Elemental_Profile_Set LumpAllProfileSets();
    void IncludeExcludeAllElements(bool value);
    void SetOMandSizeConstituents(const string &_omconstituent, const string &_sizeconsituent)
    {
        omconstituent_ = _omconstituent;
        sizeconsituent_ = _sizeconsituent;
    }
    void SetOMandSizeConstituents(const vector<string> &_omsizeconstituents)
    {
        if (_omsizeconstituents.size()==0)
            return;
        else if (_omsizeconstituents.size()==1)
            omconstituent_ = _omsizeconstituents[0];
        else if (_omsizeconstituents.size()==2)
        {
            omconstituent_ = _omsizeconstituents[0];
            sizeconsituent_ = _omsizeconstituents[1];
        }

    }
    vector<string> OMandSizeConstituents()
    {
        vector<string> out;
        out.push_back(omconstituent_);
        out.push_back(sizeconsituent_);
        return out;
    }
    void OutlierAnalysisForAll(const double &lowerthreshold=-3, const double &upperthreshold=3);
    ANOVA_info ANOVA(const string &element, bool logtransformed);
    CMBVector ANOVA(bool logtransformed);
    void IncludeExcludeElementsBasedOn(const vector<string> elements);
    SourceSinkData RandomlyEliminateSourceSamples(const double &percentage);
    CMBTimeSeriesSet BootStrap(const double &percentage, unsigned int number_of_samples, string target_sample, bool softmax_transformation);
    bool BootStrap(Results *res, const double &percentage, unsigned int number_of_samples, string target_sample, bool softmax_transformation);
    vector<string> AllSourceSampleNames() const;
    vector<string> RandomlypickSamples(const double &percentage) const;
    SourceSinkData ReplaceSourceAsTarget(const string &sourcesamplename) const;
    CMBTimeSeriesSet VerifySource(const string &sourcegroup, bool softmax_transformation, bool sizeandorganiccorrect);
    CMBTimeSeriesSet LM_Batch(transformation transform, bool om_size_correction, map<string,vector<string>> &negative_elements); //Levenberg-Marquardt batch
    Results MCMC(const string &sample, map<string,string> arguments, CMCMC<SourceSinkData> *MCMC, ProgressWindow *rtw, const string &workingfolder);
    CMBMatrix MCMC_Batch(map<string,string> arguments, CMCMC<SourceSinkData> *MCMC, ProgressWindow *rtw, const string &workingfolder);
    bool ToolsUsed(const string &toolname);
    string FirstOMConstituent();
    string FirstSizeConstituent();
    QMap<QString, double> *GetOptions() {return &options_;}

private:
      
    // ========== Element Metadata and Distributions ==========

    /// Metadata for each element (role, ratios, inclusion flags)
    map<string, element_information> element_information_;

    /// Statistical distributions of concentrations for each element across all sources
    map<string, ConcentrationSet> element_distributions_;

    // ========== Counters ==========

    /// Number of chemical elements used in analysis
    int numberofconstituents_;

    /// Number of isotope ratios used in analysis
    int numberofisotopes_;

    /// Number of source groups (excluding target/sink group)
    int numberofsourcesamplesets_;

    // ========== MCMC/Optimization Data ==========

    /// Observed values from target sample for comparison with model predictions
    vector<Observation> observations_;

    /// Parameters being optimized (contributions, distribution parameters, error terms)
    vector<Parameter> parameters_;

    // ========== File Paths ==========

    /// Output directory path for results
    string outputpath_;

    // ========== Group and Sample Identifiers ==========

    /// Name of the target/sink group (samples to be unmixed)
    string target_group_;

    /// Currently selected target sample for analysis
    string selected_target_sample_;

    // ========== Ordering Vectors ==========

    /// Ordered list of source group names
    vector<string> samplesetsorder_;

    /// All constituent names (elements + isotopes + size + OM)
    vector<string> constituent_order_;

    /// Chemical element names used in analysis
    vector<string> element_order_;

    /// Isotope ratio names used in analysis
    vector<string> isotope_order_;

    /// Particle size and organic matter constituent names
    vector<string> size_om_order_;

    // ========== Analysis Settings ==========

    /// Estimation mode (contributions only, profiles only, or both)
    estimation_mode parameter_estimation_mode_;

    /// Name of organic matter constituent (for corrections)
    string omconstituent_;

    /// Name of particle size constituent (for corrections)
    string sizeconsituent_;

    /// P-value threshold for MLR significance testing
    double regression_p_value_threshold_;

    /// Step size coefficient for gradient descent optimization
    double distance_coeff_;

    /// Error standard deviation for element concentrations (log space)
    double error_stdev_;

    /// Error standard deviation for isotope ratios (linear space)
    double error_stdev_isotope_;

    /// Small value for numerical derivative calculations
    double epsilon_;

    // ========== UI and Tracking ==========

    /// Pointer to progress window for user feedback
    ProgressWindow* rtw_;

    /// List of analysis tools/methods applied to this dataset
    list<string> tools_used_;

    /// User-configurable options (e.g., outlier thresholds)
        QMap<QString, double> options_;
    
    /**
        * @brief Retrieves the contributions from all sources
        *
        * Calculates source contributions where the first (size-2) sources have their
        * contributions stored as parameters, and the last source's contribution is
        * calculated as the remainder to ensure all contributions sum to 1.
        *
        * @return CVector A vector of size (size()-1) containing contribution values for each source.
        *         The first (size-2) elements are retrieved from stored parameters, and the last
        *         element is computed as (1 - sum of other contributions) to maintain the constraint
        *         that all contributions sum to 1.
        */
    CVector GetSourceContributions();

    /**
        * @brief Calculates the log prior probability for source contributions
        *
        * Evaluates whether the current source contributions are physically valid by checking
        * if all contributions are non-negative. This acts as a constraint in Bayesian inference.
        *
        * @return double Returns -1e10 (effectively negative infinity) if any contribution is negative,
        *         indicating an invalid state. Returns 0 if all contributions are valid (non-negative),
        *         representing a uniform prior over the valid contribution space.
        */
    double LogPriorContributions();
        
    /**
     * @brief Calculates the log-likelihood of source elemental distributions
     *
     * Computes the total log-likelihood by evaluating how well each source sample's
     * observed elemental concentrations fit their respective estimated distributions.
     * This iterates through all elements and all source groups, summing the log-probability
     * densities of each sample's elemental values under their estimated distributions.
     *
     * This is used in Bayesian inference to assess how well the model's estimated
     * distributions for each source explain the observed source sample data.
     *
     * @return double The sum of log-probabilities across all elements, all source groups,
     *         and all samples within those groups. Higher values indicate better fit
     *         between observed data and estimated distributions.
     */
    double LogLikelihoodSourceElementalDistributions();

    /**
     * @brief Calculates the log-likelihood of the model prediction versus measured data
     *
     * Compares the model's predicted elemental concentrations against the observed data
     * for the selected target sample. The likelihood is computed in log-space to handle
     * the multiplicative error structure, assuming log-normally distributed errors.
     *
     * The log-likelihood formula used is:
     * log(L) = -n*log(σ) - ||log(C_pred) - log(C_obs)||² / (2σ²)
     *
     * where n is the number of elements, σ is the error standard deviation,
     * C_pred is the predicted concentration vector, and C_obs is the observed concentration vector.
     *
     * @param est_mode Estimation mode determining how predictions are made:
     *        - elemental_profile_and_contribution: Uses fitted distributions for source profiles
     *        - other modes: Uses direct parameter values
     *
     * @return double The log-likelihood value. Returns -1e10 (effectively negative infinity)
     *         if any predicted concentration is non-positive, as log-transformation is undefined
     *         for such values. Otherwise returns the calculated log-likelihood.
     */
    double LogLikelihoodModelvsMeasured(estimation_mode est_mode = estimation_mode::elemental_profile_and_contribution);

    /**
     * @brief Calculates the log-likelihood of the model prediction versus measured isotopic data
     *
     * Compares the model's predicted isotopic delta values against the observed isotopic data
     * for the selected target sample. Unlike the elemental log-likelihood calculation, this
     * operates directly on delta notation (‰) values in linear space, assuming normally
     * distributed errors in delta values.
     *
     * The log-likelihood formula used is:
     * log(L) = -n*log(σ_iso) - ||δ_pred - δ_obs||² / (2σ_iso²)
     *
     * where n is the number of isotopes, σ_iso is the isotopic error standard deviation,
     * δ_pred is the predicted delta value vector, and δ_obs is the observed delta value vector.
     *
     * @param est_mode Estimation mode determining how predictions are made:
     *        - elemental_profile_and_contribution: Uses fitted distributions for source profiles
     *        - other modes: Uses direct parameter values
     *
     * @return double The log-likelihood value based on isotopic delta values.
     */
    
    double LogLikelihoodModelvsMeasured_Isotope(estimation_mode est_mode = estimation_mode::elemental_profile_and_contribution);
    
    Parameter* ElementalContent_mu(int element_iterator, int source_iterator);
    Parameter* ElementalContent_sigma(int element_iterator, int source_iterator);
    double ElementalContent_mu_value(int element_iterator, int source_iterator);
    double ElementalContent_sigma_value(int element_iterator, int source_iterator);
        
    void PopulateConstituentOrders();
        
    estimation_mode parameter_estimation_mode = estimation_mode::elemental_profile_and_contribution;
    list<string> tools_used;
    CMatrix BetweenGroupCovarianceMatrix();
    CMatrix WithinGroupCovarianceMatrix();
    CMatrix TotalScatterMatrix();
    double WilksLambda();
    double DFA_P_Value();
    CMBVectorSet DFA_Projected();
    CMBVectorSet DFA_Projected(const string &source1, const string &source2);
    CMBVectorSet DFA_Projected(const string &source1, SourceSinkData *original);
    CMBVector DFA_eigvector();
    CMBVector DFA_weight_vector(const string &source1, const string &source2);
    CMBVector DeviationFromMean(const string &group_name);
    CMBVector MeanElementalContent(const string &group_name);
    CMBVector MeanElementalContent();
    

};

#endif // SOURCESINKDATA_H
