#ifndef ELEMENTAL_PROFILE_SET_H
#define ELEMENTAL_PROFILE_SET_H

#include <elemental_profile.h>
#include <map>
#include <string>
#include <vector>
#include "concentrationset.h"
#include "interface.h"
#include "multiplelinearregressionset.h"
#include "resultitem.h"
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include "cmbvector.h"

/**
 * @class Elemental_Profile_Set
 * @brief Manages a collection of elemental profiles (samples) for source fingerprinting analysis
 *
 * This class stores multiple Elemental_Profile objects (one per sample) and provides:
 * - Statistical analysis across samples (distributions, covariance, correlation)
 * - Data extraction and filtering operations
 * - Organic matter and particle size corrections via multiple linear regression
 * - Box-Cox transformations and outlier detection
 * - Source contribution tracking for CMB (Chemical Mass Balance) analysis
 */
class Elemental_Profile_Set : public map<string, Elemental_Profile>, public Interface
{
public:
    // ========== Construction and Assignment ==========

    Elemental_Profile_Set();
    Elemental_Profile_Set(const Elemental_Profile_Set& other);
    Elemental_Profile_Set& operator=(const Elemental_Profile_Set& other);

    // ========== Data Management ==========

    /**
     * @brief Add a new sample profile to the set
     * @param name Sample identifier
     * @param profile Elemental profile data
     * @param elementinfo Optional element metadata for filtering
     * @return Pointer to the added profile, or nullptr if name already exists
     */
    Elemental_Profile* AppendProfile(
        const string& name,
        const Elemental_Profile& profile = Elemental_Profile(),
        map<string, element_information>* elementinfo = nullptr
    );

    /**
     * @brief Add multiple profiles from another set
     * @param profiles Source profile set
     * @param elementinfo Optional element metadata for filtering
     */
    void AppendProfiles(
        const Elemental_Profile_Set& profiles,
        map<string, element_information>* elementinfo
    );

    /**
     * @brief Rebuild element distributions from all profiles
     *
     * Call this after adding/modifying profiles to update statistical distributions
     */
    void UpdateElementDistributions();

    // ========== Data Access ==========

    /**
     * @brief Get profile by sample name (mutable)
     * @return Pointer to profile, or nullptr if not found
     */
    Elemental_Profile* GetProfile(const string& name);

    /**
     * @brief Get profile by sample name (const)
     * @return Copy of profile, or empty profile if not found
     */
    Elemental_Profile GetProfile(const string& name) const;

    /**
     * @brief Get profile by index (mutable)
     * @return Pointer to profile, or nullptr if index out of range
     */
    Elemental_Profile* GetProfile(unsigned int index);

    /**
     * @brief Get profile by index (const)
     * @return Copy of profile, or empty profile if index out of range
     */
    Elemental_Profile GetProfile(unsigned int index) const;

    /**
     * @brief Get all sample names in the set
     */
    vector<string> GetSampleNames() const;

    /**
     * @brief Get all element names (from first profile)
     */
    vector<string> GetElementNames() const;

    /**
     * @brief Get all concentration values for a specific element across samples
     */
    vector<double> GetAllConcentrationsFor(const string& element_name);

    /**
     * @brief Get all element concentrations for a specific sample
     */
    vector<double> GetConcentrationsForSample(const string& sample_name) const; 

    /**
     * @brief Check if an element exists in ALL profiles
     */
    bool ContainsElement(const string& element_name) const;

    // ========== Data Extraction and Filtering ==========

    /**
     * @brief Create subset with only specified elements
     */
    Elemental_Profile_Set ExtractElements(const vector<string>& element_list) const;

    /**
     * @brief Create subset with only chemical elements (and optionally isotopes)
     * @param elementinfo Element metadata for classification
     * @param isotopes Whether to include isotopes
     */
    Elemental_Profile_Set ExtractChemicalElements(
        const map<string, element_information>* elementinfo,
        bool isotopes
    ) const;

    /**
     * @brief Create corrected and filtered subset
     * @param exclude_samples If true, exclude samples marked as excluded
     * @param exclude_elements If true, exclude elements marked as excluded
     * @param omnsizecorrect If true, apply organic matter and size corrections
     * @param om_size Target OM and size values for correction
     * @param elementinfo Element metadata for filtering
     */
    Elemental_Profile_Set CreateCorrectedSet(
        bool exclude_samples,
        bool exclude_elements,
        bool omnsizecorrect,
        const vector<double>& om_size,
        const map<string, element_information>* elementinfo = nullptr
    ) const;

    /**
     * @brief Copy only samples marked for inclusion in analysis
     */
    Elemental_Profile_Set CopyIncludedInAnalysis(
        bool applyomsizecorrection,
        const vector<double>& om_size,
        map<string, element_information>* elementinfo = nullptr
    );

    /**
     * @brief Create subset excluding specified samples
     */
    Elemental_Profile_Set EliminateSamples(
        vector<string> samples_to_eliminate,
        map<string, element_information>* elementinfo
    ) const;

    // ========== Statistical Analysis ==========

    /**
     * @brief Get distribution for a specific element (mutable)
     */
    ConcentrationSet* GetElementDistribution(const string& element_name);

    /**
     * @brief Get distribution for a specific element (const)
     */
    ConcentrationSet GetElementDistribution(const string& element_name) const;

    /**
     * @brief Get assigned distribution type for an element
     */
    distribution_type GetAssignedDistribution(const string& element_name) const;

    /**
     * @brief Calculate mean concentration for each element
     */
    CVector CalculateElementMeans(const vector<string>& element_order = vector<string>()) const;

    /**
     * @brief Calculate covariance matrix across elements
     */
    CMBMatrix CalculateCovarianceMatrix();

    /**
     * @brief Calculate correlation matrix across elements
     */
    CMBMatrix CalculateCorrelationMatrix();

    /**
     * @brief Calculate Kolmogorov-Smirnov statistics for all elements
     */
    CMBVector CalculateKolmogorovSmirnovStatistics(distribution_type dist_type);

    /**
     * @brief Get maximum concentration across all elements and samples
     */
    double GetMaximum() const;

    /**
     * @brief Get minimum concentration across all elements and samples
     */
    double GetMinimum() const;

    // ========== Estimated Distribution Parameters (for MCMC) ==========

    /**
     * @brief Get estimated mu parameter for an element
     */
    double GetEstimatedMu(const string& element) const;

    /**
     * @brief Get estimated sigma parameter for an element
     */
    double GetEstimatedSigma(const string& element) const;

    /**
     * @brief Set estimated mu parameter for an element
     */
    bool SetEstimatedMu(const string& element, const double& value);

    /**
     * @brief Set estimated sigma parameter for an element
     */
    bool SetEstimatedSigma(const string& element, const double& value);

    /**
     * @brief Get estimated distribution for an element (for MCMC optimization)
     */
    Distribution* GetEstimatedDistribution(const string& element_name);

    /**
     * @brief Get fitted distribution for an element (from data, for likelihood)
     */
    Distribution* GetFittedDistribution(const string& element_name);

    // ========== Regression Models (OM/Size Corrections) ==========

    /**
     * @brief Build MLR models for all elements vs OM and particle size
     */
    MultipleLinearRegressionSet BuildRegressionModels(
        const string& om,
        const string& d,
        regression_form form = regression_form::linear,
        const double& p_value_threshold = 0.05
    );

    /**
     * @brief Build MLR model for single element vs OM and particle size
     */
    MultipleLinearRegression BuildRegressionModel(
        const string& element,
        const string& om,
        const string& d,
        regression_form form = regression_form::linear,
        const double& p_value_threshold = 0.05
    );

    /**
     * @brief Set regression models (build new)
     */
    void SetRegressionModels(
        const string& om,
        const string& d,
        regression_form form = regression_form::linear,
        const double& p_value_threshold = 0.05
    );

    /**
     * @brief Set regression models (copy existing)
     */
    void SetRegressionModels(const MultipleLinearRegressionSet* mlrset);

    /**
     * @brief Get internal regression model set
     */
    MultipleLinearRegressionSet* GetRegressionModels();

    /**
     * @brief Get regressions wrapped in ResultItem for display
     */
    ResultItem GetRegressionsAsResult();

    /**
     * @brief Apply OM and particle size corrections to all profiles
     */
    Elemental_Profile_Set ApplyOrganicMatterAndSizeCorrections(
        const vector<double>& om_size,
        map<string, element_information>* elementinfo = nullptr
    );

    // ========== Box-Cox Transformations ==========

    /**
     * @brief Calculate optimal Box-Cox lambda for each element
     */
    CMBVector CalculateBoxCoxParameters();

    /**
     * @brief Apply Box-Cox transformation to all profiles
     */
    Elemental_Profile_Set ApplyBoxCoxTransform(CMBVector* lambda_vals = nullptr);

    // ========== Outlier Detection ==========

    /**
     * @brief Detect outliers using Box-Cox transformation and standardization
     * @param lowerlimit Lower threshold for standardized values
     * @param upperlimit Upper threshold for standardized values
     * @return Matrix of outlier magnitudes
     */
    CMBMatrix DetectOutliers(const double& lowerlimit = 0, const double& upperlimit = 0);

    /**
     * @brief Check if outlier analysis has been performed
     */
    bool OutlierAnalysisDone() const { return outlierdone_; }

    /**
     * @brief Check for elements with negative values
     */
    vector<string> CheckForNegativeValues(const vector<string>& element_names) const;

    // ========== Source Contribution (for CMB) ==========

    /**
     * @brief Set contribution value for this source
     */
    bool SetContribution(const double& value);

    /**
     * @brief Get contribution value
     */
    double GetContribution() const { return contribution_; }

    /**
     * @brief Set softmax-transformed contribution
     */
    bool SetContributionSoftmax(const double& value);

    /**
     * @brief Get softmax-transformed contribution
     */
    double GetContributionSoftmax() const { return contribution_softmax_; }

    // ========== Utility Functions ==========

    /**
     * @brief Select top N elements across all samples (aggregate minimum)
     */
    Elemental_Profile SelectTopElementsAggregate(int n) const;

    /**
     * @brief Calculate dot product of each profile with weight vector
     */
    CMBVector CalculateDotProduct(const CVector& v) const;

    /**
     * @brief Convert to matrix format (samples × elements)
     */
    CMBMatrix ToMatrix() const;

    /**
     * @brief Convert to GSL matrix for statistical functions
     */
    gsl_matrix* CopyToGSLMatrix() const;

    // ========== Serialization ==========

    string ToString() override;
    QTableWidget* ToTable() override;
    bool writetofile(QFile* file) override;
    QJsonObject toJsonObject() override;
    bool ReadFromJsonObject(const QJsonObject& jsonobject) override;
    bool Read(const QStringList& strlist) override;

private:
    map<string, ConcentrationSet> element_distributions_;  ///< Concentration distributions for each element
    MultipleLinearRegressionSet mlr_vs_om_size_;          ///< MLR models for OM/size corrections
    double contribution_ = 0.0;                            ///< Source contribution (for CMB)
    double contribution_softmax_ = 0.0;                    ///< Softmax-transformed contribution
    bool outlierdone_ = false;                             ///< Flag indicating outlier analysis performed
};

#endif // ELEMENTAL_PROFILE_SET_H