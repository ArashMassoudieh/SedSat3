#include "sourcesinkdata.h"
#include "iostream"
#include "NormalDist.h"
#include "qjsondocument.h"
#include "resultitem.h"
#include <gsl/gsl_cdf.h>
#include "GADistribution.h"
#include "rangeset.h"
#include "QJsonArray"
#include "QJsonValue"
#include <qdir.h>



// ========== Construction and Assignment ==========

SourceSinkData::SourceSinkData()
    : map<string, Elemental_Profile_Set>(),
    element_information_(),
    element_distributions_(),
    numberofconstituents_(0),
    numberofisotopes_(0),
    numberofsourcesamplesets_(0),
    observations_(),
    outputpath_(),
    parameters_(),
    target_group_(),
    samplesetsorder_(),
    constituent_order_(),
    selected_target_sample_(),
    element_order_(),
    isotope_order_(),
    size_om_order_(),
    parameter_estimation_mode_(estimation_mode::elemental_profile_and_contribution),
    omconstituent_(),
    sizeconsituent_(),
    regression_p_value_threshold_(0.05),
    distance_coeff_(1.0),
    tools_used_(),
    options_()
{
    options_["Outlier deviation threshold"] = 3.0;
    rtw_ = nullptr;
}

SourceSinkData::SourceSinkData(const SourceSinkData& other)
    : map<string, Elemental_Profile_Set>(other),
    element_information_(other.element_information_),
    element_distributions_(other.element_distributions_),
    numberofconstituents_(other.numberofconstituents_),
    numberofisotopes_(other.numberofisotopes_),
    numberofsourcesamplesets_(other.numberofsourcesamplesets_),
    observations_(other.observations_),
    outputpath_(other.outputpath_),
    parameters_(other.parameters_),
    target_group_(other.target_group_),
    samplesetsorder_(other.samplesetsorder_),
    constituent_order_(other.constituent_order_),
    selected_target_sample_(other.selected_target_sample_),
    element_order_(other.element_order_),
    isotope_order_(other.isotope_order_),
    size_om_order_(other.size_om_order_),
    parameter_estimation_mode_(other.parameter_estimation_mode_),
    omconstituent_(other.omconstituent_),
    sizeconsituent_(other.sizeconsituent_),
    regression_p_value_threshold_(other.regression_p_value_threshold_),
    distance_coeff_(other.distance_coeff_),
    tools_used_(other.tools_used_),
    options_(other.options_)
{
    rtw_ = nullptr;
}

SourceSinkData& SourceSinkData::operator=(const SourceSinkData& other)
{
    // Check for self-assignment
    if (this == &other) {
        return *this;
    }

    // Copy base class
    map<string, Elemental_Profile_Set>::operator=(other);

    // Copy all member variables
    element_information_ = other.element_information_;
    element_distributions_ = other.element_distributions_;
    numberofconstituents_ = other.numberofconstituents_;
    numberofisotopes_ = other.numberofisotopes_;
    numberofsourcesamplesets_ = other.numberofsourcesamplesets_;
    observations_ = other.observations_;
    outputpath_ = other.outputpath_;
    parameters_ = other.parameters_;
    target_group_ = other.target_group_;
    samplesetsorder_ = other.samplesetsorder_;
    constituent_order_ = other.constituent_order_;
    selected_target_sample_ = other.selected_target_sample_;
    element_order_ = other.element_order_;
    isotope_order_ = other.isotope_order_;
    size_om_order_ = other.size_om_order_;
    parameter_estimation_mode_ = other.parameter_estimation_mode_;
    omconstituent_ = other.omconstituent_;
    sizeconsituent_ = other.sizeconsituent_;
    regression_p_value_threshold_ = other.regression_p_value_threshold_;
    distance_coeff_ = other.distance_coeff_;
    tools_used_ = other.tools_used_;
    options_ = other.options_;

    return *this;
}

SourceSinkData SourceSinkData::CreateCorrectedDataset(
    const string& target,
    bool omnsizecorrect,
    map<string, element_information>* elementinfo)
{
    SourceSinkData corrected;
    selected_target_sample_ = target;

    // Build OM and particle size vector from target sample
    vector<double> om_size;
    if (!omconstituent_.empty()) {
        om_size.push_back(at(target_group_).GetProfile(selected_target_sample_)->at(omconstituent_));
    }
    if (!sizeconsituent_.empty()) {
        om_size.push_back(at(target_group_).GetProfile(selected_target_sample_)->at(sizeconsituent_));
    }

    // Copy profile sets with appropriate corrections
    for (auto& [group_name, profile_set] : *this)
    {
        // Apply OM/size corrections to source groups, but not to target group
        bool apply_corrections = (group_name != target_group_) && omnsizecorrect;
        corrected[group_name] = profile_set.CopyIncludedInAnalysis(
            apply_corrections,
            om_size,
            elementinfo
        );
    }

    // Copy analysis settings
    corrected.omconstituent_ = omconstituent_;
    corrected.sizeconsituent_ = sizeconsituent_;
    corrected.regression_p_value_threshold_ = regression_p_value_threshold_;

    // Copy element information and distributions
    if (elementinfo)
    {
        // Filter to only include elements marked for analysis
        for (const auto& [element_name, elem_info] : element_information_)
        {
            if (elem_info.include_in_analysis &&
                elem_info.Role != element_information::role::do_not_include &&
                elem_info.Role != element_information::role::organic_carbon &&
                elem_info.Role != element_information::role::particle_size)
            {
                corrected.element_information_[element_name] = element_information_[element_name];
                corrected.element_distributions_[element_name] = element_distributions_[element_name];
            }
        }
    }
    else
    {
        // Copy all element information
        corrected.element_information_ = element_information_;
        corrected.element_distributions_ = element_distributions_;
    }

    // Copy metadata and ordering
    corrected.numberofconstituents_ = numberofconstituents_;
    corrected.numberofisotopes_ = numberofisotopes_;
    corrected.numberofsourcesamplesets_ = numberofsourcesamplesets_;
    corrected.observations_ = observations_;
    corrected.outputpath_ = outputpath_;
    corrected.target_group_ = target_group_;
    corrected.samplesetsorder_ = samplesetsorder_;
    corrected.constituent_order_ = constituent_order_;
    corrected.selected_target_sample_ = selected_target_sample_;
    corrected.element_order_ = element_order_;
    corrected.isotope_order_ = isotope_order_;
    corrected.size_om_order_ = size_om_order_;
    corrected.parameter_estimation_mode_ = parameter_estimation_mode_;
    corrected.options_ = options_;

    // Populate distributions for the corrected dataset
    corrected.PopulateElementDistributions();
    corrected.AssignAllDistributions();

    return corrected;
}

int SourceSinkData::CountElements(bool exclude_elements) const
{
    if (!exclude_elements) {
        return element_information_.size();
    }

    int count = 0;
    for (const auto& [element_name, elem_info] : element_information_)
    {
        // Include if marked for analysis AND not an excluded role
        if (elem_info.include_in_analysis &&
            elem_info.Role != element_information::role::do_not_include &&
            elem_info.Role != element_information::role::particle_size &&
            elem_info.Role != element_information::role::organic_carbon)
        {
            count++;
        }
    }

    return count;
}

SourceSinkData SourceSinkData::CreateCorrectedAndFilteredDataset(
    bool exclude_samples,
    bool exclude_elements,
    bool omnsizecorrect,
    const string& target) const
{
    SourceSinkData corrected;
    corrected.target_group_ = target_group_;

    // Set target sample (use provided or keep current)
    if (!target.empty()) {
        corrected.selected_target_sample_ = target;
    }
    else {
        corrected.selected_target_sample_ = selected_target_sample_;
    }

    // Build OM and particle size vector from target sample (if corrections enabled)
    vector<double> om_size;
    if (omnsizecorrect)
    {
        if (!omconstituent_.empty()) {
            om_size.push_back(
                at(target_group_).GetProfile(corrected.selected_target_sample_).at(omconstituent_)
            );
        }
        if (!sizeconsituent_.empty()) {
            om_size.push_back(
                at(target_group_).GetProfile(corrected.selected_target_sample_).at(sizeconsituent_)
            );
        }
    }

    // Copy and correct each profile set
    for (const auto& [group_name, profile_set] : *this)
    {
        if (group_name == target_group_)
        {
            // Target group: apply element filtering but no sample filtering or corrections
            corrected[group_name] = profile_set.CreateCorrectedSet(
                false,              // Don't exclude samples from target
                exclude_elements,   // Apply element filtering
                false,              // Don't apply OM/size corrections to target
                om_size,
                &element_information_
            );
        }
        else
        {
            // Source groups: apply all requested filtering and corrections
            corrected[group_name] = profile_set.CreateCorrectedSet(
                exclude_samples,    // Apply sample filtering
                exclude_elements,   // Apply element filtering
                omnsizecorrect,     // Apply OM/size corrections
                om_size,
                &element_information_
            );
        }
    }

    // Copy settings and metadata
    corrected.omconstituent_ = omconstituent_;
    corrected.sizeconsituent_ = sizeconsituent_;
    corrected.regression_p_value_threshold_ = regression_p_value_threshold_;
    corrected.options_ = options_;

    // Populate element information and distributions
    corrected.PopulateElementInformation(&element_information_);
    corrected.PopulateElementDistributions();
    corrected.AssignAllDistributions();

    return corrected;
}

SourceSinkData SourceSinkData::ExtractChemicalElements(bool isotopes) const
{
    SourceSinkData extracted;
    extracted.target_group_ = target_group_;

    // Extract chemical elements from each profile set
    for (const auto& [group_name, profile_set] : *this)
    {
        extracted[group_name] = profile_set.ExtractChemicalElements(&element_information_, isotopes);
    }

    // Copy settings
    extracted.omconstituent_ = omconstituent_;
    extracted.sizeconsituent_ = sizeconsituent_;
    extracted.regression_p_value_threshold_ = regression_p_value_threshold_;

    // Populate distributions for extracted elements
    extracted.PopulateElementInformation(&element_information_);
    extracted.PopulateElementDistributions();
    extracted.AssignAllDistributions();

    return extracted;
}

SourceSinkData SourceSinkData::ExtractSpecificElements(const vector<string>& element_list) const
{
    SourceSinkData extracted;

    // Extract specified elements from source groups only (not target)
    for (const auto& [group_name, profile_set] : *this)
    {
        if (group_name != target_group_)
        {
            extracted[group_name] = profile_set.ExtractElements(element_list);
        }
    }

    // Populate distributions for extracted elements
    extracted.PopulateElementInformation(&element_information_);
    extracted.PopulateElementDistributions();
    extracted.AssignAllDistributions();

    return extracted;
}

void SourceSinkData::Clear()
{
    // Clear all profile sets (base class map)
    clear();

    // Clear element data
    element_information_.clear();
    element_distributions_.clear();

    // Reset counters
    numberofconstituents_ = 0;
    numberofisotopes_ = 0;
    numberofsourcesamplesets_ = 0;

    // Clear MCMC/optimization data
    observations_.clear();
    parameters_.clear();

    // Clear identifiers and paths
    outputpath_.clear();
    target_group_.clear();
    selected_target_sample_.clear();

    // Clear ordering vectors
    samplesetsorder_.clear();
    constituent_order_.clear();
    element_order_.clear();
    isotope_order_.clear();
    size_om_order_.clear();

    // Clear tracking data
    tools_used_.clear();

    // Note: options_ is intentionally not cleared (preserves user settings)
}

Elemental_Profile_Set* SourceSinkData::AppendSampleSet(
    const string& name,
    const Elemental_Profile_Set& elemental_profile_set)
{
    // Check if group name already exists
    if (count(name) > 0)
    {
        std::cerr << "Sample set '" + name + "' already exists!" << std::endl;
        return nullptr;
    }

    // Add the new sample set
    operator[](name) = elemental_profile_set;

    return &operator[](name);
}


Elemental_Profile_Set* SourceSinkData::GetSampleSet(const string& name)
{
    if (count(name) == 0) {
        return nullptr;
    }

    return &operator[](name);
}

vector<string> SourceSinkData::GetSampleNames(const string& group_name) const
{
    const Elemental_Profile_Set* profile_set =
        (count(group_name) > 0) ? &at(group_name) : nullptr;

    if (profile_set) {
        return profile_set->GetSampleNames();
    }

    return vector<string>();
}

vector<string> SourceSinkData::GetGroupNames() const
{
    vector<string> group_names;
    group_names.reserve(size());

    for (const auto& [group_name, profile_set] : *this)
    {
        group_names.push_back(group_name);
    }

    return group_names;
}

vector<string> SourceSinkData::GetElementNames() const
{
    if (empty()) {
        return vector<string>();
    }

    // Get element names from first sample of first group
    const Elemental_Profile_Set& first_group = begin()->second;
    if (first_group.empty()) {
        return vector<string>();
    }

    const Elemental_Profile& first_sample = first_group.begin()->second;

    vector<string> element_names;
    element_names.reserve(first_sample.size());

    for (const auto& [element_name, concentration] : first_sample)
    {
        element_names.push_back(element_name);
    }

    return element_names;
}

profiles_data SourceSinkData::ExtractConcentrationData(
    const vector<vector<string>>& indicators) const
{
    profiles_data extracted;
    extracted.element_names = GetElementNames();
    extracted.values.reserve(indicators.size());
    extracted.sample_names.reserve(indicators.size());

    for (const auto& indicator : indicators)
    {
        const string& group_name = indicator[0];
        const string& sample_name = indicator[1];

        const Elemental_Profile_Set* group =
            (count(group_name) > 0) ? &at(group_name) : nullptr;

        if (group)
        {
            vector<double> concentrations = group->GetConcentrationsForSample(sample_name);
            extracted.values.push_back(concentrations);
            extracted.sample_names.push_back(sample_name);
        }
    }

    return extracted;
}

Elemental_Profile_Set SourceSinkData::ExtractSamplesAsProfileSet(
    const vector<vector<string>>& indicators) const
{
    Elemental_Profile_Set extracted;

    for (const auto& indicator : indicators)
    {
        const string& group_name = indicator[0];
        const string& sample_name = indicator[1];

        const Elemental_Profile_Set* group =
            (count(group_name) > 0) ? &at(group_name) : nullptr;

        if (group && group->count(sample_name) > 0)
        {
            const Elemental_Profile& profile = group->at(sample_name);
            // Prefix with group name to ensure uniqueness
            extracted.AppendProfile(group_name + "-" + sample_name, profile);
        }
    }

    return extracted;
}
element_data SourceSinkData::ExtractElementConcentrations(
    const string& element,
    const string& group) const
{
    element_data extracted;
    extracted.group_name = group;

    const Elemental_Profile_Set* profile_set =
        (count(group) > 0) ? &at(group) : nullptr;

    if (!profile_set) {
        return extracted;
    }

    extracted.values.reserve(profile_set->size());
    extracted.sample_names.reserve(profile_set->size());

    for (const auto& [sample_name, profile] : *profile_set)
    {
        extracted.values.push_back(profile.GetValue(element));
        extracted.sample_names.push_back(sample_name);
    }

    return extracted;
}

void SourceSinkData::PopulateElementDistributions()
{
    vector<string> element_names = GetElementNames();
    element_distributions_.clear();

    // Update distributions within each group, then aggregate
    for (auto& [group_name, profile_set] : *this)
    {
        // Build distributions within this group
        profile_set.UpdateElementDistributions();

        // Aggregate into overall distributions
        for (const string& element : element_names)
        {
            element_distributions_[element].AppendSet(
                profile_set.GetElementDistribution(element)
            );
        }
    }
}

map<string, vector<double>> SourceSinkData::ExtractElementDataByGroup(const string& element) const
{
    map<string, vector<double>> extracted;
    vector<string> group_names = GetGroupNames();

    for (const string& group_name : group_names)
    {
        element_data group_data = ExtractElementConcentrations(element, group_name);
        extracted[group_name] = group_data.values;
    }

    return extracted;
}


void SourceSinkData::AssignAllDistributions()
{
    vector<string> element_names = GetElementNames();

    for (const string& element : element_names)
    {
        // Assign distribution at dataset level (all groups combined)
        Distribution* overall_fitted = element_distributions_[element].GetFittedDistribution();
        overall_fitted->distribution = element_distributions_[element].SelectBestDistribution();
        overall_fitted->parameters = element_distributions_[element].EstimateDistributionParameters();

        // Assign distributions at group level
        for (auto& [group_name, profile_set] : *this)
        {
            ConcentrationSet* group_dist = profile_set.GetElementDistribution(element);
            Distribution* group_fitted = group_dist->GetFittedDistribution();

            // Use same distribution type as overall
            group_fitted->distribution = overall_fitted->distribution;

            // Estimate parameters from group-specific data
            if (element_information_[element].Role != element_information::role::isotope)
            {
                // Elements: use selected distribution type
                group_fitted->parameters = group_dist->EstimateDistributionParameters(
                    overall_fitted->distribution
                );
            }
            else
            {
                // Isotopes: always use normal distribution
                group_fitted->parameters = group_dist->EstimateDistributionParameters(
                    distribution_type::normal
                );
            }
        }
    }
}

Distribution* SourceSinkData::GetFittedDistribution(const string& element_name)
{
    if (element_distributions_.count(element_name) == 0) {
        return nullptr;
    }

    return element_distributions_[element_name].GetFittedDistribution();
}

void SourceSinkData::PopulateElementInformation(const map<string, element_information>* ElementInfo)
{
    element_information_.clear();
    vector<string> element_names = GetElementNames();

    for (const string& element : element_names)
    {
        if (ElementInfo == nullptr)
        {
            // Use default element information
            element_information_[element] = element_information();
        }
        else
        {
            // Copy from provided element information
            element_information_[element] = ElementInfo->at(element);
        }
    }

    // Update source sample set count
    if (!target_group_.empty())
    {
        numberofsourcesamplesets_ = size() - 1;  // Exclude target group
    }
    else
    {
        numberofsourcesamplesets_ = size();      // No target group
    }
}

string SourceSinkData::GetParameterName(int index) const
{
    if (parameter(index))
    {
        return parameter(index)->Name();
    }
    return "";
}

bool SourceSinkData::InitializeContributionsRandomly()
{
    // Set first source to dummy value to force recalculation
    GetSampleSet(GetSourceOrder()[0])->SetContribution(1.2);

    // Keep sampling until valid contributions found (sum = 1, all >= 0)
    while (GetContributionVector(false).min() < 0 || GetContributionVector(false).sum() > 1)
    {
        for (size_t i = 0; i < samplesetsorder_.size() - 1; i++)
        {
            SetContribution(i, unitrandom());
        }
    }

    return true;
}

bool SourceSinkData::InitializeContributionsRandomlySoftmax()
{
    // Set first source to dummy value
    GetSampleSet(GetSourceOrder()[0])->SetContribution(1.2);

    // Generate random normal values
    CVector X = getnormal(samplesetsorder_.size(), 0, 1).vec;

    // Apply softmax transformation to ensure sum = 1
    SetContributionSoftmax(X);

    return true;
}


bool SourceSinkData::InitializeParametersAndObservations(
    const string& targetsamplename,
    estimation_mode est_mode)
{
    // Populate element ordering vectors
    PopulateConstituentOrders();
    selected_target_sample_ = targetsamplename;

    // Validate data is loaded
    if (empty())
    {
        std::cerr << "Data has not been loaded!" << std::endl;
        return false;
    }

    numberofsourcesamplesets_ = size() - 1;

    // Clear existing parameters and observations
    parameters_.clear();
    observations_.clear();
    samplesetsorder_.clear();

    // ========== Setup Source Contribution Parameters ==========

    if (est_mode != estimation_mode::source_elemental_profiles_based_on_source_data)
    {
        // Create contribution parameters for all sources except last (calculated from constraint)
        for (auto& [group_name, profile_set] : *this)
        {
            if (group_name != GetTargetGroup())
            {
                Parameter p;
                p.SetName(group_name + "_contribution");
                p.SetPriorDistribution(distribution_type::dirichlet);
                p.SetRange(0, 1);
                parameters_.push_back(p);
                samplesetsorder_.push_back(group_name);
            }
        }
        // Remove last contribution parameter (calculated from sum constraint)
        parameters_.pop_back();
    }
    else
    {
        // Build source order without creating parameters
        for (auto& [group_name, profile_set] : *this)
        {
            if (group_name != GetTargetGroup())
            {
                samplesetsorder_.push_back(group_name);
            }
        }
    }

    // ========== Count Elements and Isotopes ==========

    numberofconstituents_ = 0;
    numberofisotopes_ = 0;

    for (const auto& [element_name, elem_info] : element_information_)
    {
        if (elem_info.include_in_analysis)
        {
            if (elem_info.Role == element_information::role::element) {
                numberofconstituents_++;
            }
            else if (elem_info.Role == element_information::role::isotope) {
                numberofisotopes_++;
            }
        }
    }

    // ========== Initialize Estimated Distributions from Fitted ==========

    // For elements
    for (const auto& [element_name, elem_info] : element_information_)
    {
        if (elem_info.Role == element_information::role::element && elem_info.include_in_analysis)
        {
            for (auto& [group_name, profile_set] : *this)
            {
                if (group_name != target_group_)
                {
                    // Copy fitted distribution to estimated (starting point for optimization)
                    *profile_set.GetEstimatedDistribution(element_name) =
                        *profile_set.GetFittedDistribution(element_name);
                }
            }
        }
    }

    // For isotopes
    for (const auto& [element_name, elem_info] : element_information_)
    {
        if (elem_info.Role == element_information::role::isotope && elem_info.include_in_analysis)
        {
            for (auto& [group_name, profile_set] : *this)
            {
                if (group_name != target_group_)
                {
                    *profile_set.GetEstimatedDistribution(element_name) =
                        *profile_set.GetFittedDistribution(element_name);
                }
            }
        }
    }

    // ========== Setup Source Profile Parameters (if estimating profiles) ==========

    if (est_mode != estimation_mode::only_contributions)
    {
        // Mu parameters for elements (lognormal)
        for (const auto& [element_name, elem_info] : element_information_)
        {
            if (elem_info.Role == element_information::role::element && elem_info.include_in_analysis)
            {
                for (auto& [group_name, profile_set] : *this)
                {
                    if (group_name != target_group_)
                    {
                        Parameter p;
                        p.SetName(group_name + "_" + element_name + "_mu");
                        p.SetPriorDistribution(distribution_type::normal);

                        // Set estimated distribution type
                        profile_set.GetEstimatedDistribution(element_name)->SetType(
                            distribution_type::lognormal
                        );

                        // Set parameter range around fitted mu
                        const Distribution* fitted = profile_set.GetFittedDistribution(element_name);
                        p.SetRange(fitted->parameters[0] - 0.2, fitted->parameters[0] + 0.2);

                        parameters_.push_back(p);
                    }
                }
            }
        }

        // Mu parameters for isotopes (normal)
        for (const auto& [element_name, elem_info] : element_information_)
        {
            if (elem_info.Role == element_information::role::isotope && elem_info.include_in_analysis)
            {
                for (auto& [group_name, profile_set] : *this)
                {
                    if (group_name != target_group_)
                    {
                        Parameter p;
                        p.SetName(group_name + "_" + element_name + "_mu");
                        p.SetPriorDistribution(distribution_type::normal);

                        // Set estimated distribution type
                        profile_set.GetEstimatedDistribution(element_name)->SetType(
                            distribution_type::normal
                        );

                        // Set parameter range around fitted mu
                        const Distribution* fitted = profile_set.GetFittedDistribution(element_name);
                        p.SetRange(fitted->parameters[0] - 0.2, fitted->parameters[0] + 0.2);

                        parameters_.push_back(p);
                    }
                }
            }
        }

        // Sigma parameters for elements
        for (const auto& [element_name, elem_info] : element_information_)
        {
            if (elem_info.Role == element_information::role::element && elem_info.include_in_analysis)
            {
                for (auto& [group_name, profile_set] : *this)
                {
                    if (group_name != target_group_)
                    {
                        Parameter p;
                        p.SetName(group_name + "_" + element_name + "_sigma");
                        p.SetPriorDistribution(distribution_type::lognormal);

                        // Set parameter range around fitted sigma (with bounds)
                        const Distribution* fitted = profile_set.GetFittedDistribution(element_name);
                        double lower = std::max(fitted->parameters[1] * 0.8, 0.001);
                        double upper = std::max(fitted->parameters[1] / 0.8, 2.0);
                        p.SetRange(lower, upper);

                        parameters_.push_back(p);
                    }
                }
            }
        }

        // Sigma parameters for isotopes
        for (const auto& [element_name, elem_info] : element_information_)
        {
            if (elem_info.Role == element_information::role::isotope && elem_info.include_in_analysis)
            {
                for (auto& [group_name, profile_set] : *this)
                {
                    if (group_name != target_group_)
                    {
                        Parameter p;
                        p.SetName(group_name + "_" + element_name + "_sigma");
                        p.SetPriorDistribution(distribution_type::lognormal);

                        // Set parameter range around fitted sigma (with bounds)
                        const Distribution* fitted = profile_set.GetFittedDistribution(element_name);
                        double lower = std::max(fitted->parameters[1] * 0.8, 0.001);
                        double upper = std::max(fitted->parameters[1] / 0.8, 2.0);
                        p.SetRange(lower, upper);

                        parameters_.push_back(p);
                    }
                }
            }
        }
    }

    // ========== Setup Error and Observation Parameters ==========

    if (est_mode != estimation_mode::source_elemental_profiles_based_on_source_data)
    {
        // Error standard deviation for elements
        Parameter error_param;
        error_param.SetName("Error STDev");
        error_param.SetPriorDistribution(distribution_type::lognormal);
        error_param.SetRange(0.01, 0.1);
        parameters_.push_back(error_param);

        // Create observations for elements
        for (const auto& [element_name, elem_info] : element_information_)
        {
            if (elem_info.Role == element_information::role::element && elem_info.include_in_analysis)
            {
                Observation obs;
                obs.SetName(targetsamplename + "_" + element_name);
                obs.AppendValues(0, GetElementalProfile(targetsamplename)->GetValue(element_name));
                observations_.push_back(obs);
            }
        }

        // Error standard deviation for isotopes
        Parameter error_param_isotope;
        error_param_isotope.SetName("Error STDev for isotopes");
        error_param_isotope.SetPriorDistribution(distribution_type::lognormal);
        error_param_isotope.SetRange(0.01, 0.1);
        parameters_.push_back(error_param_isotope);

        // Create observations for isotopes
        for (const auto& [element_name, elem_info] : element_information_)
        {
            if (elem_info.Role == element_information::role::isotope && elem_info.include_in_analysis)
            {
                Observation obs;
                obs.SetName(targetsamplename + "_" + element_name);
                obs.AppendValues(0, GetElementalProfile(targetsamplename)->GetValue(element_name));
                observations_.push_back(obs);
            }
        }
    }

    return true;
}


CVector SourceSinkData::GetSourceContributions()
{
    CVector contributions(size()-1);
    for (unsigned long int i=0; i<size()-2; i++)
    {
        contributions[i] = parameter(i)->Value();
    }
    contributions[size()-2] = 1 - contributions.sum();
    return contributions;
}


double SourceSinkData::LogPriorContributions()
{
    if (GetSourceContributions().min()<0)
        return -1e10;
    else
        return 0;
}

double SourceSinkData::LogLikelihoodSourceElementalDistributions()
{
    double logLikelihood = 0;
    for (unsigned int element_counter=0; element_counter<element_order_.size(); element_counter++)
    {
        for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets_; source_group_counter++)
        {
            Elemental_Profile_Set *this_source_group = GetSampleSet(samplesetsorder_[source_group_counter]);


            for (map<string,Elemental_Profile>::iterator sample = this_source_group->begin(); sample!=this_source_group->end(); sample++)
            {
                logLikelihood += this_source_group->GetElementDistribution(element_order_[element_counter])->GetEstimatedDistribution()->EvalLog(sample->second.GetValue(element_order_[element_counter]));
            }

        }
    }
    return logLikelihood;
}

CVector SourceSinkData::ObservedDataforSelectedSample(const string &SelectedTargetSample)
{
    CVector observed_data(element_order_.size());
    for (unsigned int i=0; i<element_order_.size(); i++)
    {   if (SelectedTargetSample!="")
            observed_data[i] = this->GetSampleSet(GetTargetGroup())->GetProfile(SelectedTargetSample)->GetValue(element_order_[i]);
        else if (selected_target_sample_!="")
            observed_data[i] = this->GetSampleSet(GetTargetGroup())->GetProfile(selected_target_sample_)->GetValue(element_order_[i]);
    }
    return observed_data;
}

CVector SourceSinkData::ObservedDataforSelectedSample_Isotope(const string &SelectedTargetSample)
{
    CVector observed_data(isotope_order_.size());
    for (unsigned int i=0; i<isotope_order_.size(); i++)
    {   string corresponding_element = element_information_[isotope_order_[i]].base_element;
        if (SelectedTargetSample!="")
            observed_data[i] = (this->GetSampleSet(GetTargetGroup())->GetProfile(SelectedTargetSample)->GetValue(isotope_order_[i])/double(1000)+1.0)*element_information_[isotope_order_[i]].standard_ratio*GetSampleSet(GetTargetGroup())->GetProfile(SelectedTargetSample)->GetValue(corresponding_element);
        else if (selected_target_sample_!="")
            observed_data[i] = (this->GetSampleSet(GetTargetGroup())->GetProfile(selected_target_sample_)->GetValue(isotope_order_[i])/double(1000)+1.0)*element_information_[isotope_order_[i]].standard_ratio*GetSampleSet(GetTargetGroup())->GetProfile(SelectedTargetSample)->GetValue(corresponding_element);
    }
    return observed_data;
}

CVector SourceSinkData::ObservedDataforSelectedSample_Isotope_delta(const string &SelectedTargetSample)
{
    CVector observed_data(isotope_order_.size());
    for (unsigned int i=0; i<isotope_order_.size(); i++)
    {   string corresponding_element = element_information_[isotope_order_[i]].base_element;
        if (SelectedTargetSample!="")
            observed_data[i] = this->GetSampleSet(GetTargetGroup())->GetProfile(SelectedTargetSample)->GetValue(isotope_order_[i]);
        else if (selected_target_sample_!="")
            observed_data[i] = this->GetSampleSet(GetTargetGroup())->GetProfile(selected_target_sample_)->GetValue(isotope_order_[i]);
    }
    return observed_data;
}


double SourceSinkData::LogLikelihoodModelvsMeasured(estimation_mode est_mode)
{
    // Determine parameter mode based on estimation mode
    parameter_mode param_mode = (est_mode == estimation_mode::elemental_profile_and_contribution)
        ? parameter_mode::based_on_fitted_distribution
        : parameter_mode::direct;

    // Get predicted and observed concentrations
    CVector predicted_concentrations = PredictTarget(param_mode);
    CVector observed_concentrations = ObservedDataforSelectedSample(selected_target_sample_);

    // Check validity of predictions (all values must be positive for log-transformation)
    if (predicted_concentrations.min() <= 0)
    {
        return -1e10; // Invalid prediction: return extremely low likelihood
    }

    // Calculate log-likelihood in log-space
    // Formula: log(L) = -n*log(σ) - ||log(C_pred) - log(C_obs)||² / (2σ²)
    const double num_elements = predicted_concentrations.num;
    const double normalization_term = num_elements * log(error_stdev_);

    CVector log_residuals = predicted_concentrations.Log() - observed_concentrations.Log();
    const double sum_squared_residuals = pow(log_residuals.norm2(), 2);
    const double variance_term = 2.0 * pow(error_stdev_, 2);

    double log_likelihood = -normalization_term - (sum_squared_residuals / variance_term);

    return log_likelihood;
}

double SourceSinkData::LogLikelihoodModelvsMeasured_Isotope(estimation_mode est_mode)
{
    // Determine parameter mode based on estimation mode
    parameter_mode param_mode = (est_mode == estimation_mode::elemental_profile_and_contribution)
        ? parameter_mode::based_on_fitted_distribution
        : parameter_mode::direct;

    // Get predicted and observed isotopic delta values
    CVector predicted_deltas = PredictTarget_Isotope_delta(param_mode);
    CVector observed_deltas = ObservedDataforSelectedSample_Isotope_delta(selected_target_sample_);

    // Calculate log-likelihood in linear space (delta values)
    // Formula: log(L) = -n*log(σ_iso) - ||δ_pred - δ_obs||² / (2σ_iso²)
    const double num_isotopes = predicted_deltas.num;
    const double normalization_term = num_isotopes * log(error_stdev_isotope_);

    CVector residuals = predicted_deltas - observed_deltas;
    const double sum_squared_residuals = pow(residuals.norm2(), 2);
    const double variance_term = 2.0 * pow(error_stdev_isotope_, 2);

    double log_likelihood = -normalization_term - (sum_squared_residuals / variance_term);

    return log_likelihood;
}


CVector SourceSinkData::ResidualVector()
{
    // Get predicted values using direct parameter mode
    CVector predicted_concentrations = PredictTarget(parameter_mode::direct);
    CVector predicted_deltas = PredictTarget_Isotope_delta(parameter_mode::direct);

    // Check for invalid predictions
    if (!predicted_concentrations.is_finite())
    {
        qDebug() << "Warning: Non-finite predicted concentrations detected in ResidualVector()";
    }

    // Get observed values for the selected target sample
    CVector observed_concentrations = ObservedDataforSelectedSample(selected_target_sample_);
    CVector observed_deltas = ObservedDataforSelectedSample_Isotope_delta(selected_target_sample_);

    // Calculate residuals
    // Elemental: log-space residuals (log(predicted) - log(observed))
    CVector elemental_residuals = predicted_concentrations.Log() - observed_concentrations.Log();

    // Isotopic: linear residuals (predicted_delta - observed_delta)
    CVector isotopic_residuals = predicted_deltas - observed_deltas;

    // Combine residuals: [elemental_residuals, isotopic_residuals]
    CVector combined_residuals = elemental_residuals;
    combined_residuals.append(isotopic_residuals);

    // Check for non-finite residuals (indicates numerical issues)
    if (!combined_residuals.is_finite())
    {
        qDebug() << "Warning: Non-finite residuals detected in ResidualVector()";
        qDebug() << "Contribution vector:" << GetContributionVector().toString();
        qDebug() << "Contribution vector (softmax):" << GetContributionVectorSoftmax().toString();
    }

    return combined_residuals;
}

CVector_arma SourceSinkData::ResidualVector_arma()
{
    // Get predicted values (returns CVector with .vec member for Armadillo compatibility)
    CVector_arma predicted_concentrations = PredictTarget().vec;
    CVector_arma predicted_deltas = PredictTarget_Isotope_delta().vec;

    // Get observed values for the selected target sample
    CVector_arma observed_concentrations = ObservedDataforSelectedSample(selected_target_sample_).vec;
    CVector_arma observed_deltas = ObservedDataforSelectedSample_Isotope_delta(selected_target_sample_).vec;

    // Calculate residuals
    // Elemental: log-space residuals (log(predicted) - log(observed))
    CVector_arma elemental_residuals = predicted_concentrations.Log() - observed_concentrations.Log();

    // Isotopic: linear residuals (predicted_delta - observed_delta)
    CVector_arma isotopic_residuals = predicted_deltas - observed_deltas;

    // Combine residuals: [elemental_residuals, isotopic_residuals]
    CVector_arma combined_residuals = elemental_residuals;
    combined_residuals.append(isotopic_residuals);

    return combined_residuals;
}


CMatrix_arma SourceSinkData::ResidualJacobian_arma()
{
    const size_t num_sources = GetSourceOrder().size();
    const size_t num_parameters = num_sources - 1; // Last contribution is implicit
    const size_t num_residuals = element_order_.size() + isotope_order_.size();

    CMatrix_arma jacobian(num_parameters, num_residuals);

    // Store base state
    CVector_arma base_contributions = GetContributionVector(false).vec;
    CVector_arma base_residuals = ResidualVector_arma();

    // Compute derivatives using finite differences
    for (unsigned int i = 0; i < num_parameters; i++)
    {
        // Adaptive epsilon: smaller when far from 0.5, larger when near boundaries
        const double epsilon = (0.5 - base_contributions[i]) * 1e-6;

        // Perturb contribution
        SetContribution(i, base_contributions[i] + epsilon);
        CVector_arma perturbed_residuals = ResidualVector_arma();

        // Compute derivative: ∂residuals/∂contribution_i
        jacobian.setcol(i, (perturbed_residuals - base_residuals) / epsilon);

        // Restore original contribution
        SetContribution(i, base_contributions[i]);
    }

    return jacobian;
}

CMatrix SourceSinkData::ResidualJacobian()
{
    const size_t num_sources = GetSourceOrder().size();
    const size_t num_parameters = num_sources - 1; // Last contribution is implicit
    const size_t num_residuals = element_order_.size() + isotope_order_.size();

    CMatrix jacobian(num_parameters, num_residuals);

    // Store base state
    CVector base_contributions = GetContributionVector(false);
    CVector base_residuals = ResidualVector();

    // Compute derivatives using finite differences
    for (unsigned int i = 0; i < num_parameters; i++)
    {
        // Adaptive epsilon: smaller when far from 0.5, larger when near boundaries
        const double epsilon = (0.5 - base_contributions[i]) * 1e-3;

        // Perturb contribution
        SetContribution(i, base_contributions[i] + epsilon);
        CVector perturbed_residuals = ResidualVector();

        // Compute derivative: ∂residuals/∂contribution_i
        jacobian.setrow(i, (perturbed_residuals - base_residuals) / epsilon);

        // Restore original contribution
        SetContribution(i, base_contributions[i]);
    }

    return jacobian;
}

CMatrix SourceSinkData::ResidualJacobian_softmax()
{
    const size_t num_sources = GetSourceOrder().size();
    const size_t num_residuals = element_order_.size() + isotope_order_.size();

    CMatrix jacobian(num_sources, num_residuals); // All sources are parameters in softmax

    // Store base state
    CVector base_softmax_params = GetContributionVectorSoftmax();
    CVector base_residuals = ResidualVector();

    // Compute derivatives using finite differences
    for (unsigned int i = 0; i < num_sources; i++)
    {
        // Sign-dependent epsilon for better numerical behavior
        const double epsilon = -sign(base_softmax_params[i]) * 1e-3;

        // Perturb softmax parameter
        CVector perturbed_params = base_softmax_params;
        perturbed_params[i] += epsilon;
        SetContributionSoftmax(perturbed_params);

        CVector perturbed_residuals = ResidualVector();

        // Compute derivative: ∂residuals/∂softmax_param_i
        jacobian.setrow(i, (perturbed_residuals - base_residuals) / epsilon);

        // Restore original softmax parameters
        SetContributionSoftmax(base_softmax_params);
    }

    return jacobian;
}

CVector SourceSinkData::OneStepLevenberg_Marquardt(double lambda)
{
    // Get current residuals and Jacobian
    CVector residuals = ResidualVector();
    CMatrix jacobian = ResidualJacobian();

    // Compute J^T J (normal equations matrix)
    CMatrix jacobian_transpose_jacobian = jacobian * Transpose(jacobian);

    // Apply Levenberg-Marquardt damping: (1 + λ) on diagonal
    jacobian_transpose_jacobian.ScaleDiagonal(1.0 + lambda);

    // Compute right-hand side: J^T r
    CVector jacobian_times_residuals = jacobian * residuals;

    // Check for near-singular matrix and add regularization if needed
    if (det(jacobian_transpose_jacobian) <= 1e-6)
    {
        // Add additional regularization: λI
        const size_t matrix_size = jacobian_transpose_jacobian.getnumcols();
        CMatrix identity = CMatrix::Diag(matrix_size);
        jacobian_transpose_jacobian += lambda * identity;
    }

    // Solve for parameter update: dx = (J^T J)^(-1) J^T r
    CVector parameter_update = jacobian_times_residuals / jacobian_transpose_jacobian;

    return parameter_update;
}

CVector SourceSinkData::OneStepLevenberg_Marquardt_softmax(double lambda)
{
    // Get current residuals and Jacobian (softmax parameterization)
    CVector residuals = ResidualVector();
    CMatrix jacobian = ResidualJacobian_softmax();

    // Compute J^T J (normal equations matrix)
    CMatrix jacobian_transpose_jacobian = jacobian * Transpose(jacobian);

    // Apply Levenberg-Marquardt damping: (1 + λ) on diagonal
    jacobian_transpose_jacobian.ScaleDiagonal(1.0 + lambda);

    // Compute right-hand side: J^T r
    CVector jacobian_times_residuals = jacobian * residuals;

    // Check for near-singular matrix and add regularization if needed
    if (det(jacobian_transpose_jacobian) <= 1e-6)
    {
        // Add additional regularization: λI
        const size_t matrix_size = jacobian_transpose_jacobian.getnumcols();
        CMatrix identity = CMatrix::Diag(matrix_size);
        jacobian_transpose_jacobian += lambda * identity;
    }

    // Solve for parameter update: dx = (J^T J)^(-1) J^T r
    CVector parameter_update = jacobian_times_residuals / jacobian_transpose_jacobian;

    return parameter_update;
}

bool SourceSinkData::SolveLevenberg_Marquardt(transformation trans)
{
    // Initialize contributions based on parameterization
    if (trans == transformation::linear)
        InitializeContributionsRandomly();
    else if (trans == transformation::softmax)
        InitializeContributionsRandomlySoftmax();

    // Algorithm parameters
    const double tolerance = 1e-10;
    const int max_iterations = 1000;
    const double improvement_threshold = 0.8; // Error must reduce to <80% for lambda decrease
    const double lambda_decrease_factor = 1.2;
    const double lambda_increase_factor = 1.2;
    const double lambda_no_update_factor = 5.0;

    // Initialize tracking variables
    double lambda = 1.0;
    double current_error = ResidualVector().norm2();
    double previous_error = 1000.0;
    double initial_param_change = 10000.0;
    double current_param_change = 10000.0;
    int iteration = 0;

    // Main optimization loop
    while (current_error > tolerance &&
        current_param_change > tolerance &&
        iteration < max_iterations)
    {
        // Store current parameter values
        CVector current_params;
        if (trans == transformation::linear)
            current_params = GetContributionVector(false);
        else if (trans == transformation::softmax)
            current_params = GetContributionVectorSoftmax();

        previous_error = current_error;

        // Compute parameter update step
        CVector parameter_update;
        if (trans == transformation::linear)
            parameter_update = OneStepLevenberg_Marquardt(lambda);
        else if (trans == transformation::softmax)
            parameter_update = OneStepLevenberg_Marquardt_softmax(lambda);

        // Handle singular Jacobian (no valid update computed)
        if (parameter_update.num == 0)
        {
            lambda *= lambda_no_update_factor;
            continue; // Skip to next iteration
        }

        // Track parameter change magnitude
        current_param_change = parameter_update.norm2();
        if (iteration == 0)
            initial_param_change = current_param_change;

        // Apply parameter update
        CVector updated_params = current_params - parameter_update;
        if (trans == transformation::linear)
            SetContribution(updated_params);
        else if (trans == transformation::softmax)
            SetContributionSoftmax(updated_params);

        // Evaluate new error
        CVector residuals = ResidualVector();
        current_error = residuals.norm2();

        // Adaptive lambda adjustment based on error change
        if (current_error < previous_error * improvement_threshold)
        {
            // Good progress: decrease lambda (move toward Gauss-Newton)
            lambda /= lambda_decrease_factor;
        }
        else if (current_error > previous_error)
        {
            // Error increased: reject step, increase lambda (move toward gradient descent)
            lambda *= lambda_increase_factor;

            // Restore previous parameters
            if (trans == transformation::linear)
                SetContribution(current_params);
            else if (trans == transformation::softmax)
                SetContributionSoftmax(current_params);

            current_error = previous_error;
        }
        // else: modest improvement, keep lambda unchanged

        // Update progress visualization if available
        if (rtw_)
        {
            rtw_->AppendPoint(iteration, current_error);
            rtw_->SetXRange(0, iteration);
            rtw_->SetProgress(1.0 - current_param_change / initial_param_change);
        }

        iteration++;
    }

    // TODO: Return convergence status instead of always false
    return false;
}


CVector SourceSinkData::PredictTarget(parameter_mode param_mode)
{
    // Compute predicted concentrations: C = Source_Matrix × Contribution_Vector
    CMatrix source_mean_matrix = BuildSourceMeanMatrix(param_mode);
    CVector contribution_vector = GetContributionVector();
    CVector predicted_concentrations = source_mean_matrix * contribution_vector;

    // Update stored predicted values in observation objects for each element
    for (unsigned int i = 0; i < element_order_.size(); i++)
    {
        observation(i)->SetPredictedValue(predicted_concentrations[i]);
    }

    return predicted_concentrations;
}

CVector SourceSinkData::PredictTarget_Isotope(parameter_mode param_mode)
{
    // Compute predicted isotopic concentrations: C = Source_Matrix × Contribution_Vector
    CMatrix source_isotope_matrix = BuildSourceMeanMatrix_Isotopes(param_mode);
    CVector contribution_vector = GetContributionVector();
    CVector predicted_isotope_concentrations = source_isotope_matrix * contribution_vector;

    return predicted_isotope_concentrations;
}

CVector SourceSinkData::PredictTarget_Isotope_delta(parameter_mode param_mode)
{
    CMatrix SourceMeanMat = BuildSourceMeanMatrix(param_mode);
    CMatrix SourceMeanMat_Iso = BuildSourceMeanMatrix_Isotopes(param_mode);
    CVector C_elements = SourceMeanMat*GetContributionVector();
    CVector C = SourceMeanMat_Iso*GetContributionVector();
    for (unsigned int i=0; i<numberofisotopes_; i++)
    {
        string corresponding_element = element_information_[isotope_order_[i]].base_element;
        double predicted_corresponding_element_concentration = C_elements[lookup(element_order_,corresponding_element)];
        double ratio = C[i]/predicted_corresponding_element_concentration;
        double standard_ratio = element_information_[isotope_order_[i]].standard_ratio;
        C[i] = (ratio/standard_ratio-1.0)*1000.0;
    }
    for (unsigned int i=element_order_.size(); i<element_order_.size()+isotope_order_.size(); i++)
    {
        observation(i)->SetPredictedValue(C[i-element_order_.size()]);
    }
    return C;
}


double SourceSinkData::GetObjectiveFunctionValue()
{
    return -LogLikelihood(parameter_estimation_mode_);
}

double SourceSinkData::LogLikelihood(estimation_mode est_mode)
{
    // Initialize likelihood components
    double source_data_log_likelihood = 0.0;
    double element_observation_log_likelihood = 0.0;
    double isotope_observation_log_likelihood = 0.0;

    // Component 1: Log-likelihood of source data given estimated distributions
    // P(Y | μ, σ) - Only included when estimating source profiles
    if (est_mode != estimation_mode::only_contributions) {
        source_data_log_likelihood = LogLikelihoodSourceElementalDistributions();
    }

    // Component 2: Log-prior on contributions
    // P(f) - Always included (uniform Dirichlet prior)
    double contribution_log_prior = LogPriorContributions();

    // Component 3: Log-likelihood of observations given model predictions
    // P(C_obs | f, μ, σ) - Only when fitting to target sample
    if (est_mode != estimation_mode::source_elemental_profiles_based_on_source_data) {
        // Elements: log P(C_obs | C_pred)
        element_observation_log_likelihood = LogLikelihoodModelvsMeasured(est_mode);

        // Isotopes: log P(δ_obs | δ_pred)
        isotope_observation_log_likelihood = LogLikelihoodModelvsMeasured_Isotope(est_mode);
    }

    // Sum all components to get total log-likelihood
    double total_log_likelihood = source_data_log_likelihood +
        element_observation_log_likelihood +
        isotope_observation_log_likelihood +
        contribution_log_prior;

    return total_log_likelihood;
}

CMatrix SourceSinkData::BuildSourceMeanMatrix(parameter_mode param_mode)
{
    const size_t num_elements = element_order_.size();
    const size_t num_sources = numberofsourcesamplesets_;

    // Initialize source mean matrix: rows = elements, cols = sources
    // Matrix entry [i,j] = mean concentration of element i in source j
    CMatrix source_means(num_elements, num_sources);

    // Fill matrix with mean concentrations for each element and source
    for (size_t element_idx = 0; element_idx < num_elements; element_idx++)
    {
        const string& element_name = element_order_[element_idx];

        for (size_t source_idx = 0; source_idx < num_sources; source_idx++)
        {
            const string& source_name = samplesetsorder_[source_idx];
            Elemental_Profile_Set* source_group = GetSampleSet(source_name);

            // Get estimated distribution for this element in this source
            Distribution* element_dist = source_group->GetEstimatedDistribution(element_name);

            // Calculate mean based on parameter mode
            if (param_mode == parameter_mode::based_on_fitted_distribution) {
                // Parametric mean from distribution parameters (μ, σ)
                // For lognormal: Mean = exp(μ + σ²/2)
                source_means[element_idx][source_idx] = element_dist->Mean();
            }
            else {
                // Empirical mean calculated directly from data points
                source_means[element_idx][source_idx] = element_dist->DataMean();
            }
        }
    }

    return source_means;
}

CMatrix SourceSinkData::BuildSourceMeanMatrix_Isotopes(parameter_mode param_mode)
{
    const size_t num_isotopes = isotope_order_.size();
    const size_t num_sources = numberofsourcesamplesets_;

    // Initialize source isotope matrix: rows = isotopes, cols = sources
    // Matrix entry [i,j] = mean absolute concentration of isotope i in source j
    CMatrix source_isotope_means(num_isotopes, num_sources);

    // Fill matrix with mean isotope concentrations (converted from delta)
    for (size_t isotope_idx = 0; isotope_idx < num_isotopes; isotope_idx++)
    {
        const string& isotope_name = isotope_order_[isotope_idx];
        const element_information& isotope_info = element_information_[isotope_name];
        const string& base_element = isotope_info.base_element;
        const double standard_ratio = isotope_info.standard_ratio;

        for (size_t source_idx = 0; source_idx < num_sources; source_idx++)
        {
            const string& source_name = samplesetsorder_[source_idx];
            Elemental_Profile_Set* source_group = GetSampleSet(source_name);

            // Get distributions for isotope and its base element
            Distribution* isotope_dist = source_group->GetEstimatedDistribution(isotope_name);
            Distribution* base_element_dist = source_group->GetEstimatedDistribution(base_element);

            // Retrieve mean values based on parameter mode
            double mean_delta;
            double mean_base_concentration;

            if (param_mode == parameter_mode::based_on_fitted_distribution) {
                // Use parametric means from fitted distributions
                mean_delta = isotope_dist->Mean();
                mean_base_concentration = base_element_dist->Mean();
            }
            else {
                // Use empirical means from actual data
                mean_delta = isotope_dist->DataMean();
                mean_base_concentration = base_element_dist->DataMean();
            }

            // Convert delta notation to absolute concentration
            // Formula: [isotope] = (δ/1000 + 1) × R_standard × [base_element]
            double delta_ratio = (mean_delta / 1000.0) + 1.0;
            double absolute_isotope_concentration = delta_ratio * standard_ratio * mean_base_concentration;

            source_isotope_means[isotope_idx][source_idx] = absolute_isotope_concentration;
        }
    }

    return source_isotope_means;
}

CVector SourceSinkData::GetContributionVector(bool include_all)
{
    // Determine vector size based on whether to include constrained contribution
    const size_t num_contributions = include_all ?
        numberofsourcesamplesets_ :
        numberofsourcesamplesets_ - 1;

    CVector contributions(num_contributions);

    // Retrieve contribution values from each source group
    for (size_t source_idx = 0; source_idx < num_contributions; source_idx++)
    {
        const string& source_name = samplesetsorder_[source_idx];
        Elemental_Profile_Set* source_group = GetSampleSet(source_name);
        contributions[source_idx] = source_group->GetContribution();
    }

    return contributions;
}

CVector SourceSinkData::GetContributionVectorSoftmax()
{
    const size_t num_sources = numberofsourcesamplesets_;
    CVector softmax_parameters(num_sources);

    // Retrieve softmax parameter values from each source group
    for (size_t source_idx = 0; source_idx < num_sources; source_idx++)
    {
        const string& source_name = samplesetsorder_[source_idx];
        Elemental_Profile_Set* source_group = GetSampleSet(source_name);
        softmax_parameters[source_idx] = source_group->GetContributionSoftmax();
    }

    return softmax_parameters;
}




void SourceSinkData::SetContribution(size_t source_index, double contribution_value)
{
    // Set the specified contribution
    const string& source_name = samplesetsorder_[source_index];
    GetSampleSet(source_name)->SetContribution(contribution_value);

    // Update last contribution to maintain sum constraint
    const size_t last_source_index = samplesetsorder_.size() - 1;
    const string& last_source_name = samplesetsorder_[last_source_index];

    // Calculate constrained contribution: c_last = 1 - Σ(c_i)
    double sum_of_independent = GetContributionVector(false).sum();
    double constrained_contribution = 1.0 - sum_of_independent;

    GetSampleSet(last_source_name)->SetContribution(constrained_contribution);
}

void SourceSinkData::SetContributionSoftmax(size_t source_index, double softmax_value)
{
    // Set softmax parameter (no constraint to maintain)
    const string& source_name = samplesetsorder_[source_index];
    GetSampleSet(source_name)->SetContributionSoftmax(softmax_value);
}

void SourceSinkData::SetContribution(const CVector& contributions)
{
    // Set each contribution (last contribution updated automatically each time)
    for (size_t i = 0; i < static_cast<size_t>(contributions.num); i++)
    {
        SetContribution(i, contributions.vec[i]);
    }

    // Validate: all contributions should be non-negative
    if (contributions.min() < 0.0) {
        std::cerr << "Warning: Negative contribution detected in SetContribution()" << std::endl;
        std::cerr << "  Min value: " << contributions.min() << std::endl;
    }
}

void SourceSinkData::SetContributionSoftmax(const CVector& softmax_params)
{
    // Compute softmax normalization denominator: Σ exp(x_i)
    double denominator = 0.0;
    for (size_t i = 0; i < static_cast<size_t>(softmax_params.num); i++)
    {
        denominator += exp(softmax_params[i]);
    }

    // Apply softmax transformation: c_i = exp(x_i) / Σ exp(x_j)
    for (size_t i = 0; i < static_cast<size_t>(softmax_params.num); i++)
    {
        double softmax_param = softmax_params.at(i);
        double contribution = exp(softmax_param) / denominator;

        // Set both softmax parameter and computed contribution
        SetContributionSoftmax(i, softmax_param);
        SetContribution(i, contribution);
    }

    // Validate: resulting contributions should be valid
    CVector resulting_contributions = GetContributionVector();
    if (resulting_contributions.min() < 0.0) {
        std::cerr << "Warning: Invalid contributions after softmax transformation" << std::endl;
        std::cerr << "  Min contribution: " << resulting_contributions.min() << std::endl;
        std::cerr << "  This should not happen with softmax - check for numerical issues" << std::endl;
    }
}

Parameter* SourceSinkData::GetElementDistributionMuParameter(
    size_t element_index,
    size_t source_index)
{
    // Parameter vector layout:
    // [0 to n-2]: Contribution parameters (n-1 independent contributions)
    // [n-1 onwards]: Element μ parameters

    const size_t num_contribution_params = size() - 1;  // Exclude target group
    const size_t element_mu_base_index = num_contribution_params;

    // Calculate index for this element's μ in this source
    // Elements are stored in blocks: all sources for element 0, then all sources for element 1, etc.
    const size_t parameter_index = element_mu_base_index +
        (element_index * numberofsourcesamplesets_) +
        source_index;

    // Bounds check
    if (parameter_index >= parameters_.size()) {
        std::cerr << "Error: Parameter index out of bounds in GetElementDistributionMuParameter" << std::endl;
        return nullptr;
    }

    return &parameters_[parameter_index];
}

Parameter* SourceSinkData::GetElementDistributionSigmaParameter(
    size_t element_index,
    size_t source_index)
{
    // Parameter vector layout:
    // [0 to n-2]: Contribution parameters
    // [n-1 to n-1 + num_elements × num_sources - 1]: Element μ parameters
    // [next block]: Element σ parameters ← We're here

    const size_t num_contribution_params = size() - 1;
    const size_t num_element_mu_params = numberofconstituents_ * numberofsourcesamplesets_;
    const size_t element_sigma_base_index = num_contribution_params + num_element_mu_params;

    // Calculate index for this element's σ in this source
    const size_t parameter_index = element_sigma_base_index +
        (element_index * numberofsourcesamplesets_) +
        source_index;

    // Bounds check
    if (parameter_index >= parameters_.size()) {
        std::cerr << "Error: Parameter index out of bounds in GetElementDistributionSigmaParameter" << std::endl;
        return nullptr;
    }

    return &parameters_[parameter_index];
}

double SourceSinkData::GetElementDistributionMuValue(
    size_t element_index,
    size_t source_index)
{
    Parameter* mu_param = GetElementDistributionMuParameter(element_index, source_index);

    if (mu_param == nullptr) {
        std::cerr << "Warning: Unable to retrieve μ parameter for element "
            << element_index << ", source " << source_index << std::endl;
        return 0.0;
    }

    return mu_param->Value();
}

double SourceSinkData::GetElementDistributionSigmaValue(
    size_t element_index,
    size_t source_index)
{
    Parameter* sigma_param = GetElementDistributionSigmaParameter(element_index, source_index);

    if (sigma_param == nullptr) {
        std::cerr << "Warning: Unable to retrieve σ parameter for element "
            << element_index << ", source " << source_index << std::endl;
        return 0.0;
    }

    return sigma_param->Value();
}

bool SourceSinkData::SetParameterValue(size_t index, double value)
{
    // Validate index
    if (index >= parameters_.size()) {
        return false;
    }

    // Determine which parameter blocks are active based on estimation mode
    const bool estimating_contributions =
        (parameter_estimation_mode_ != estimation_mode::source_elemental_profiles_based_on_source_data);
    const bool estimating_profiles =
        (parameter_estimation_mode_ != estimation_mode::only_contributions);

    // Calculate parameter block boundaries
    const size_t num_contribution_params = estimating_contributions ? (numberofsourcesamplesets_ - 1) : 0;
    const size_t num_element_mu_params = numberofconstituents_ * numberofsourcesamplesets_;
    const size_t num_isotope_mu_params = numberofisotopes_ * numberofsourcesamplesets_;
    const size_t num_element_sigma_params = num_element_mu_params;
    const size_t num_isotope_sigma_params = num_isotope_mu_params;

    // Update parameter value
    parameters_[index].SetValue(value);

    // ========== Block 1: Contribution Parameters ==========
    if (estimating_contributions && index < num_contribution_params)
    {
        // Update this source's contribution
        GetSampleSet(samplesetsorder_[index])->SetContribution(value);

        // Update last contribution to maintain sum constraint: c_n = 1 - Σ(c_1...c_{n-1})
        const size_t last_source_index = numberofsourcesamplesets_ - 1;
        double sum_of_independent = GetContributionVector(false).sum();
        GetSampleSet(samplesetsorder_[last_source_index])->SetContribution(1.0 - sum_of_independent);

        return true;
    }

    // ========== Block 2: Element μ Parameters ==========
    size_t element_mu_start = num_contribution_params;
    size_t element_mu_end = element_mu_start + num_element_mu_params;

    if (estimating_profiles &&
        parameter_estimation_mode_ == estimation_mode::elemental_profile_and_contribution &&
        index >= element_mu_start && index < element_mu_end)
    {
        size_t offset = index - element_mu_start;
        size_t element_index = offset / numberofsourcesamplesets_;
        size_t source_index = offset % numberofsourcesamplesets_;

        GetElementDistribution(element_order_[element_index], samplesetsorder_[source_index])
            ->SetEstimatedMu(value);

        return true;
    }

    // ========== Block 3: Isotope μ Parameters ==========
    size_t isotope_mu_start = element_mu_end;
    size_t isotope_mu_end = isotope_mu_start + num_isotope_mu_params;

    if (estimating_profiles &&
        parameter_estimation_mode_ == estimation_mode::elemental_profile_and_contribution &&
        index >= isotope_mu_start && index < isotope_mu_end)
    {
        size_t offset = index - isotope_mu_start;
        size_t isotope_index = offset / numberofsourcesamplesets_;
        size_t source_index = offset % numberofsourcesamplesets_;

        GetElementDistribution(isotope_order_[isotope_index], samplesetsorder_[source_index])
            ->SetEstimatedMu(value);

        return true;
    }

    // ========== Block 4: Element σ Parameters ==========
    size_t element_sigma_start = isotope_mu_end;
    size_t element_sigma_end = element_sigma_start + num_element_sigma_params;

    if (estimating_profiles &&
        parameter_estimation_mode_ == estimation_mode::elemental_profile_and_contribution &&
        index >= element_sigma_start && index < element_sigma_end)
    {
        if (value < 0.0) {
            std::cerr << "Error: Element σ parameter cannot be negative (value = "
                << value << ")" << std::endl;
            return false;
        }

        size_t offset = index - element_sigma_start;
        size_t element_index = offset / numberofsourcesamplesets_;
        size_t source_index = offset % numberofsourcesamplesets_;

        GetElementDistribution(element_order_[element_index], samplesetsorder_[source_index])
            ->SetEstimatedSigma(value);

        return true;
    }

    // ========== Block 5: Isotope σ Parameters ==========
    size_t isotope_sigma_start = element_sigma_end;
    size_t isotope_sigma_end = isotope_sigma_start + num_isotope_sigma_params;

    if (estimating_profiles &&
        parameter_estimation_mode_ == estimation_mode::elemental_profile_and_contribution &&
        index >= isotope_sigma_start && index < isotope_sigma_end)
    {
        if (value < 0.0) {
            std::cerr << "Error: Isotope σ parameter cannot be negative (value = "
                << value << ")" << std::endl;
            return false;
        }

        size_t offset = index - isotope_sigma_start;
        size_t isotope_index = offset / numberofsourcesamplesets_;
        size_t source_index = offset % numberofsourcesamplesets_;

        GetElementDistribution(isotope_order_[isotope_index], samplesetsorder_[source_index])
            ->SetEstimatedSigma(value);

        return true;
    }

    // ========== Block 6: Element Error Standard Deviation ==========
    size_t error_element_index = isotope_sigma_end;

    if (estimating_contributions && index == error_element_index)
    {
        if (value < 0.0) {
            std::cerr << "Error: Element error std dev cannot be negative (value = "
                << value << ")" << std::endl;
            return false;
        }

        error_stdev_ = value;
        return true;
    }

    // ========== Block 7: Isotope Error Standard Deviation ==========
    size_t error_isotope_index = error_element_index + 1;

    if (estimating_contributions && index == error_isotope_index)
    {
        if (value < 0.0) {
            std::cerr << "Error: Isotope error std dev cannot be negative (value = "
                << value << ")" << std::endl;
            return false;
        }

        error_stdev_isotope_ = value;
        return true;
    }

    // If we reach here, index doesn't match any known parameter block
    std::cerr << "Warning: Parameter index " << index << " not recognized" << std::endl;
    return false;
}

double SourceSinkData::GetParameterValue(size_t index) const
{
    return parameters_[index].Value();
}

bool SourceSinkData::SetParameterValue(const CVector& values)
{
    bool all_successful = true;

    // Update each parameter sequentially
    for (size_t i = 0; i < static_cast<size_t>(values.num); i++)
    {
        bool success = SetParameterValue(i, values[i]);
        all_successful &= success;

        // Note: Continue even if one fails to attempt all updates
    }

    return all_successful;
}

CVector SourceSinkData::GetParameterValue() const
{
    CVector parameter_values(parameters_.size());

    // Extract value from each parameter
    for (size_t i = 0; i < parameters_.size(); i++)
    {
        parameter_values[i] = parameters_[i].Value();
    }

    return parameter_values;
}

CVector SourceSinkData::Gradient(const CVector& parameters, estimation_mode est_mode)
{
    CVector gradient(parameters.num);
    CVector perturbed_params = parameters;

    // Set base parameters and evaluate baseline log-likelihood
    SetParameterValue(parameters);
    double baseline_log_likelihood = LogLikelihood(est_mode);

    // Compute partial derivative for each parameter using finite differences
    for (size_t i = 0; i < static_cast<size_t>(parameters.num); i++)
    {
        // Perturb parameter i by epsilon
        perturbed_params[i] += epsilon_;
        SetParameterValue(perturbed_params);

        // Evaluate log-likelihood at perturbed point
        double perturbed_log_likelihood = LogLikelihood(est_mode);

        // Compute numerical derivative: ∂log(L)/∂θ_i ≈ Δlog(L) / Δθ_i
        gradient[i] = (perturbed_log_likelihood - baseline_log_likelihood) / epsilon_;

        // Restore parameter for next iteration
        perturbed_params[i] = parameters[i];
    }

    // Normalize gradient to unit length for numerical stability
    return gradient / gradient.norm2();
}

CVector SourceSinkData::GradientUpdate(estimation_mode est_mode)
{
    // Get current parameters and evaluate baseline likelihood
    CVector current_params = GetParameterValue();
    double baseline_likelihood = LogLikelihood(est_mode);

    // Compute normalized gradient direction
    CVector gradient_direction = Gradient(current_params, est_mode);

    // Try two candidate steps: standard and aggressive (2×)
    CVector candidate_step1 = current_params + distance_coeff_ * gradient_direction;
    SetParameterValue(candidate_step1);
    double likelihood_step1 = LogLikelihood(est_mode);

    CVector candidate_step2 = current_params + 2.0 * distance_coeff_ * gradient_direction;
    SetParameterValue(candidate_step2);
    double likelihood_step2 = LogLikelihood(est_mode);

    // Debug output
    std::cout << "Distance Coefficient: " << distance_coeff_ << std::endl;

    // Reset step size if it becomes too small
    if (distance_coeff_ < 1e-6) {
        distance_coeff_ = 1.0;
    }

    // ========== Strategy 1: Aggressive step (2×) is best ==========
    if (likelihood_step2 > likelihood_step1 && likelihood_step2 > baseline_likelihood)
    {
        // Success with larger step - increase step size for next iteration
        distance_coeff_ *= 2.0;
        return candidate_step2;
    }

    // ========== Strategy 2: Standard step (1×) is best ==========
    else if (likelihood_step1 > likelihood_step2 && likelihood_step1 > baseline_likelihood)
    {
        // Success with standard step - keep current step size
        SetParameterValue(candidate_step1);
        return candidate_step1;
    }

    // ========== Strategy 3: Neither improved - backtrack ==========
    else
    {
        const int max_backtrack_iterations = 5;
        int backtrack_count = 0;

        // Progressively reduce step size until we find improvement
        while (baseline_likelihood >= likelihood_step1 && backtrack_count < max_backtrack_iterations)
        {
            distance_coeff_ *= 0.5;  // Halve the step size

            candidate_step1 = current_params + distance_coeff_ * gradient_direction;
            SetParameterValue(candidate_step1);
            likelihood_step1 = LogLikelihood(est_mode);

            backtrack_count++;
        }

        if (backtrack_count < max_backtrack_iterations)
        {
            // Found an improvement with smaller step
            SetParameterValue(candidate_step1);
            return candidate_step1;
        }
        else
        {
            // No improvement found after backtracking - return to original
            std::cerr << "Warning: No improvement found after " << max_backtrack_iterations
                << " backtracking iterations" << std::endl;
            SetParameterValue(current_params);
            return current_params;
        }
    }
}

vector<string> SourceSinkData::ElementsToBeUsedInCMB()
{
    // Reset counters and ordering
    numberofconstituents_ = 0;
    constituent_order_.clear();

    // Collect all chemical elements (exclude isotopes, size, organic carbon)
    for (const auto& [element_name, elem_info] : element_information_)
    {
        if (elem_info.Role == element_information::role::element)
        {
            numberofconstituents_++;
            constituent_order_.push_back(element_name);
        }
    }

    return constituent_order_;
}

vector<string> SourceSinkData::IsotopesToBeUsedInCMB()
{
    // Reset counters and ordering
    numberofisotopes_ = 0;
    isotope_order_.clear();

    // Collect isotopes marked for inclusion in analysis
    for (const auto& [element_name, elem_info] : element_information_)
    {
        if (elem_info.Role == element_information::role::isotope &&
            elem_info.include_in_analysis)
        {
            numberofisotopes_++;
            isotope_order_.push_back(element_name);
        }
    }

    return isotope_order_;
}
void SourceSinkData::PopulateConstituentOrders()
{
    // Reset all counters and ordering vectors
    numberofconstituents_ = 0;
    constituent_order_.clear();
    element_order_.clear();
    size_om_order_.clear();
    isotope_order_.clear();

    // Pass 1: Collect ALL constituents (elements, isotopes, metadata)
    for (const auto& [constituent_name, constituent_info] : element_information_)
    {
        numberofconstituents_++;
        constituent_order_.push_back(constituent_name);
    }

    // Pass 2: Collect chemical elements included in analysis
    for (const auto& [element_name, elem_info] : element_information_)
    {
        if (elem_info.Role == element_information::role::element &&
            elem_info.include_in_analysis)
        {
            element_order_.push_back(element_name);
        }
    }

    // Pass 3: Collect isotopes included in analysis
    for (const auto& [isotope_name, elem_info] : element_information_)
    {
        if (elem_info.Role == element_information::role::isotope &&
            elem_info.include_in_analysis)
        {
            isotope_order_.push_back(isotope_name);
        }
    }

    // Pass 4: Collect particle size parameters
    for (const auto& [param_name, elem_info] : element_information_)
    {
        if (elem_info.Role == element_information::role::particle_size)
        {
            size_om_order_.push_back(param_name);
        }
    }

    // Pass 5: Collect organic carbon parameters
    for (const auto& [param_name, elem_info] : element_information_)
    {
        if (elem_info.Role == element_information::role::organic_carbon)
        {
            size_om_order_.push_back(param_name);
        }
    }
}
vector<string> SourceSinkData::SourceGroupNames() const
{
    vector<string> source_names;

    // Collect all group names except the target
    for (const auto& [group_name, profile_set] : *this)
    {
        if (group_name != target_group_)
        {
            source_names.push_back(group_name);
        }
    }

    return source_names;
}

Elemental_Profile* SourceSinkData::GetElementalProfile(const string& sample_name)
{
    // Search all groups for the specified sample
    for (auto& [group_name, profile_set] : *this)
    {
        // Search within this group's samples
        for (const auto& [profile_name, profile] : profile_set)
        {
            if (profile_name == sample_name)
            {
                // Found it - return pointer to the profile
                return profile_set.GetProfile(profile_name);
            }
        }
    }

    // Sample not found in any group
    return nullptr;
}

ResultItem SourceSinkData::GetContribution()
{
    ResultItem result;
    Contribution* contributions = new Contribution();

    // Package contribution values with source names
    vector<string> source_order = GetSourceOrder();
    CVector contribution_values = GetContributionVector();

    for (size_t i = 0; i < source_order.size(); i++)
    {
        (*contributions)[source_order[i]] = contribution_values[i];
    }

    // Configure result item
    result.SetName("Contributions");
    result.SetResult(contributions);
    result.SetType(result_type::contribution);

    return result;
}

ResultItem SourceSinkData::GetPredictedElementalProfile(parameter_mode param_mode)
{
    ResultItem result;
    Elemental_Profile* predicted_profile = new Elemental_Profile();

    // Compute predicted concentrations using mixing model
    CVector predicted_concentrations = PredictTarget(param_mode);
    vector<string> element_names = ElementOrder();

    // Package predictions into elemental profile
    for (size_t i = 0; i < element_names.size(); i++)
    {
        predicted_profile->AppendElement(element_names[i], predicted_concentrations[i]);
    }

    // Configure result item
    result.SetName("Modeled Elemental Profile");
    result.SetResult(predicted_profile);
    result.SetType(result_type::predicted_concentration);

    return result;
}

CVector SourceSinkData::GetPredictedValues()
{
    const size_t num_observations = ObservationsCount();
    CVector predicted_values(num_observations);

    // Collect predicted value from each observation
    for (size_t i = 0; i < num_observations; i++)
    {
        predicted_values[i] = observation(i)->PredictedValue();
    }

    return predicted_values;
}

ResultItem SourceSinkData::GetPredictedElementalProfile_Isotope(parameter_mode param_mode)
{
    ResultItem result;
    Elemental_Profile* predicted_isotopes = new Elemental_Profile();

    // Compute predicted delta values using mixing model
    CVector predicted_delta_values = PredictTarget_Isotope_delta(param_mode);
    vector<string> isotope_names = IsotopeOrder();

    // Package predictions into elemental profile
    for (size_t i = 0; i < isotope_names.size(); i++)
    {
        predicted_isotopes->AppendElement(isotope_names[i], predicted_delta_values[i]);
    }

    // Configure result item
    result.SetName("Modeled Elemental Profile for Isotopes");
    result.SetResult(predicted_isotopes);
    result.SetType(result_type::predicted_concentration);

    return result;
}

ResultItem SourceSinkData::GetObservedvsModeledElementalProfile(parameter_mode param_mode)
{
    ResultItem result;

    // Get predicted and observed profiles
    Elemental_Profile* predicted = static_cast<Elemental_Profile*>(
        GetPredictedElementalProfile(param_mode).Result()
        );
    Elemental_Profile* observed = static_cast<Elemental_Profile*>(
        GetObservedElementalProfile().Result()
        );

    // Create comparison profile set
    Elemental_Profile_Set* comparison = new Elemental_Profile_Set();
    comparison->AppendProfile("Observed", *observed);
    comparison->AppendProfile("Modeled", *predicted);

    // Configure result item
    result.SetName("Observed vs Modeled Elemental Profile");
    result.SetResult(comparison);
    result.SetType(result_type::elemental_profile_set);

    return result;
}

vector<ResultItem> SourceSinkData::GetMLRResults()
{
    vector<ResultItem> mlr_results;

    // Collect regression results from each sample group
    for (auto& [group_name, profile_set] : *this)
    {
        // Get MLR models for this group
        ResultItem regression_result = profile_set.GetRegressionsAsResult();

        // Configure for display
        regression_result.SetShowTable(true);
        regression_result.SetName("OM & Size MLR for " + group_name);

        mlr_results.push_back(regression_result);
    }

    return mlr_results;
}

ResultItem SourceSinkData::GetObservedvsModeledElementalProfile_Isotope(parameter_mode param_mode)
{
    ResultItem result;

    // Get predicted and observed isotope profiles
    Elemental_Profile* predicted = static_cast<Elemental_Profile*>(
        GetPredictedElementalProfile_Isotope(param_mode).Result()
        );
    Elemental_Profile* observed = static_cast<Elemental_Profile*>(
        GetObservedElementalProfile_Isotope().Result()
        );

    // Create comparison profile set
    Elemental_Profile_Set* comparison = new Elemental_Profile_Set();
    comparison->AppendProfile("Observed", *observed);
    comparison->AppendProfile("Modeled", *predicted);

    // Configure result item
    result.SetName("Observed vs Modeled Elemental Profile for Isotopes");
    result.SetResult(comparison);
    result.SetType(result_type::elemental_profile_set);

    return result;
}

ResultItem SourceSinkData::GetObservedElementalProfile()
{
    ResultItem result;
    Elemental_Profile* observed_profile = new Elemental_Profile();

    // Retrieve observed concentrations for selected target sample
    CVector observed_concentrations = ObservedDataforSelectedSample(selected_target_sample_);
    vector<string> element_names = ElementOrder();

    // Package observations into elemental profile
    for (size_t i = 0; i < element_names.size(); i++)
    {
        observed_profile->AppendElement(element_names[i], observed_concentrations[i]);
    }

    // Configure result item
    result.SetName("Observed Elemental Profile");
    result.SetResult(observed_profile);
    result.SetType(result_type::predicted_concentration);

    return result;
}

ResultItem SourceSinkData::GetObservedElementalProfile_Isotope()
{
    ResultItem result;
    Elemental_Profile* observed_isotopes = new Elemental_Profile();

    // Retrieve observed delta values for selected target sample
    CVector observed_delta_values = ObservedDataforSelectedSample_Isotope_delta(selected_target_sample_);
    vector<string> isotope_names = IsotopeOrder();

    // Package observations into elemental profile
    for (size_t i = 0; i < isotope_names.size(); i++)
    {
        observed_isotopes->AppendElement(isotope_names[i], observed_delta_values[i]);
    }

    // Configure result item
    result.SetName("Observed Elemental Profile for Isotopes");
    result.SetResult(observed_isotopes);
    result.SetType(result_type::predicted_concentration);

    return result;
}

ResultItem SourceSinkData::GetCalculatedElementMeans()
{
    Elemental_Profile_Set* profile_set = new Elemental_Profile_Set();

    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != end(); it++)
    {
        if (it->first != target_group_)
        {
            Elemental_Profile element_profile;
            for (unsigned int element_counter = 0; element_counter < element_order_.size(); element_counter++)
            {
                element_profile.AppendElement(element_order_[element_counter], it->second.GetElementDistribution(element_order_[element_counter])->CalculateMean());
            }
            profile_set->AppendProfile(it->first, element_profile);
        }
    }

    ResultItem resitem;
    resitem.SetName("Calculated mean elemental contents");
    resitem.SetType(result_type::elemental_profile_set);
    resitem.SetResult(profile_set);
    return resitem;
}

ResultItem SourceSinkData::GetCalculatedElementSigma()
{
    Elemental_Profile_Set* profile_set = new Elemental_Profile_Set();

    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != end(); it++)
    {
        if (it->first != target_group_)
        {
            Elemental_Profile element_profile;
            for (unsigned int element_counter = 0; element_counter < element_order_.size(); element_counter++)
            {
                if (it->second.GetElementDistribution(element_order_[element_counter])->GetEstimatedDistribution()->distribution == distribution_type::normal)
                    element_profile.AppendElement(element_order_[element_counter], it->second.GetElementDistribution(element_order_[element_counter])->CalculateStdDev());
                else if (it->second.GetElementDistribution(element_order_[element_counter])->GetEstimatedDistribution()->distribution == distribution_type::lognormal)
                    element_profile.AppendElement(element_order_[element_counter], it->second.GetElementDistribution(element_order_[element_counter])->CalculateStdDevLog());
            }
            profile_set->AppendProfile(it->first, element_profile);
        }
    }

    ResultItem resitem;
    resitem.SetName("Calculated elemental contents standard deviations");
    resitem.SetType(result_type::elemental_profile_set);
    resitem.SetResult(profile_set);
    return resitem;
}


vector<ResultItem> SourceSinkData::GetSourceProfiles()
{
    vector<ResultItem> source_profile_results;

    // Package elemental profiles for each source group
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != end(); it++)
    {
        // Skip target group - only process sources
        if (it->first != target_group_)
        {
            // Create and configure result item for this source
            ResultItem result_item;
            result_item.SetName("Elemental Profiles for " + it->first);
            result_item.SetShowAsString(true);
            result_item.SetShowTable(true);
            result_item.SetShowGraph(true);
            result_item.SetType(result_type::elemental_profile_set);

            // Copy profile set for this source
            Elemental_Profile_Set* profile_set = new Elemental_Profile_Set();
            *profile_set = it->second;
            result_item.SetResult(profile_set);

            source_profile_results.push_back(result_item);
        }
    }

    return source_profile_results;
}


ResultItem SourceSinkData::GetCalculatedElementMu()
{
    Elemental_Profile_Set* profile_set = new Elemental_Profile_Set();

    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != end(); it++)
    {
        if (it->first != target_group_)
        {
            Elemental_Profile element_profile;
            for (unsigned int element_counter = 0; element_counter < element_order_.size(); element_counter++)
            {
                element_profile.AppendElement(element_order_[element_counter], it->second.GetElementDistribution(element_order_[element_counter])->CalculateMeanLog());
            }
            profile_set->AppendProfile(it->first, element_profile);
        }
    }

    ResultItem resitem;
    resitem.SetName("Calculated mu parameter of elemental contents");
    resitem.SetType(result_type::elemental_profile_set);
    resitem.SetResult(profile_set);
    return resitem;
}

ResultItem SourceSinkData::GetEstimatedElementMu()
{
    Elemental_Profile_Set* profile_set = new Elemental_Profile_Set();

    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != end(); it++)
    {
        if (it->first != target_group_)
        {
            Elemental_Profile element_profile;
            for (unsigned int element_counter = 0; element_counter < element_order_.size(); element_counter++)
            {
                element_profile.AppendElement(element_order_[element_counter], it->second.GetElementDistribution(element_order_[element_counter])->GetEstimatedMu());
            }
            profile_set->AppendProfile(it->first, element_profile);
        }
    }

    ResultItem resitem;
    resitem.SetName("Infered mu parameter elemental contents");
    resitem.SetType(result_type::elemental_profile_set);
    resitem.SetResult(profile_set);
    return resitem;
}

ResultItem SourceSinkData::GetEstimatedElementMean()
{
    Elemental_Profile_Set* profile_set = new Elemental_Profile_Set();

    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != end(); it++)
    {
        if (it->first != target_group_)
        {
            Elemental_Profile element_profile;
            for (unsigned int element_counter = 0; element_counter < element_order_.size(); element_counter++)
            {
                double sigma = it->second.GetElementDistribution(element_order_[element_counter])->GetEstimatedSigma();
                double mu = it->second.GetElementDistribution(element_order_[element_counter])->GetEstimatedMu();
                element_profile.AppendElement(element_order_[element_counter], exp(mu + pow(sigma, 2) / 2));
            }
            profile_set->AppendProfile(it->first, element_profile);
        }
    }

    ResultItem resitem;
    resitem.SetName("Infered mean of elemental contents");
    resitem.SetType(result_type::elemental_profile_set);
    resitem.SetResult(profile_set);
    return resitem;
}

ResultItem SourceSinkData::GetEstimatedElementSigma()
{
    Elemental_Profile_Set* profile_set = new Elemental_Profile_Set();

    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != end(); it++)
    {
        if (it->first != target_group_)
        {
            Elemental_Profile element_profile;
            for (unsigned int element_counter = 0; element_counter < element_order_.size(); element_counter++)
            {
                double sigma = it->second.GetElementDistribution(element_order_[element_counter])->GetEstimatedSigma();
                element_profile.AppendElement(element_order_[element_counter], sigma);
            }
            profile_set->AppendProfile(it->first, element_profile);
        }
    }

    ResultItem resitem;
    resitem.SetName("Infered sigma parameter");
    resitem.SetType(result_type::elemental_profile_set);
    resitem.SetResult(profile_set);
    return resitem;
}



QJsonArray SourceSinkData::ToolsUsedToJsonObject()
{
    QJsonArray tools_used_json_array;
    
    // Serialize each tool name
    for (list<string>::iterator it = tools_used_.begin(); it != tools_used_.end(); it++)
    {
        tools_used_json_array.append(QString::fromStdString(*it));
    }
    
    return tools_used_json_array;
}

QJsonObject SourceSinkData::OptionsToJsonObject()
{
    QJsonObject json_object;
    
    // Serialize each option name-value pair
    for (QMap<QString, double>::iterator it = options_.begin(); it != options_.end(); it++)
    {
        json_object[it.key()] = it.value();
    }
    
    return json_object;
}

void SourceSinkData::AddtoToolsUsed(const string& tool)
{
    // Only add if not already present
    if (!ToolsUsed(tool))
    {
        tools_used_.push_back(tool);
    }
}

bool SourceSinkData::ReadToolsUsedFromJsonObject(const QJsonArray& jsonarray)
{
    // Deserialize each tool name
    foreach (const QJsonValue& value, jsonarray)
    {
        AddtoToolsUsed(value.toString().toStdString());
    }
    
    return true;
}

bool SourceSinkData::ReadElementInformationfromJsonObject(const QJsonObject& jsonobject)
{
    // Clear existing element information
    element_information_.clear();
    
    // Deserialize metadata for each element
    for (QString key : jsonobject.keys())
    {
        element_information elem_info;
        elem_info.Role = Role(jsonobject[key].toObject()["Role"].toString());
        elem_info.standard_ratio = jsonobject[key].toObject()["Standard Ratio"].toDouble();
        elem_info.base_element = jsonobject[key].toObject()["Base Element"].toString().toStdString();
        elem_info.include_in_analysis = jsonobject[key].toObject()["Include"].toBool();
        
        element_information_[key.toStdString()] = elem_info;
    }
    
    return true;
}

bool SourceSinkData::ReadElementDatafromJsonObject(const QJsonObject& jsonobject)
{
    // Clear existing data
    clear();
    
    // Deserialize each sample group
    for (QString key : jsonobject.keys())
    {
        Elemental_Profile_Set elemental_profile_set;
        elemental_profile_set.ReadFromJsonObject(jsonobject[key].toObject());
        operator[](key.toStdString()) = elemental_profile_set;
    }
    
    return true;
}

bool SourceSinkData::ReadOptionsfromJsonObject(const QJsonObject& jsonobject)
{
    // Deserialize each option
    for (QString key : jsonobject.keys())
    {
        options_[key] = jsonobject[key].toDouble();
    }
    
    return true;
}

QJsonObject SourceSinkData::ElementDataToJsonObject()
{
    QJsonObject json_object;
    
    // Serialize each sample group
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != end(); it++)
    {
        json_object[QString::fromStdString(it->first)] = it->second.toJsonObject();
    }
    
    return json_object;
}

bool SourceSinkData::WriteDataToFile(QFile* file)
{
    // Write header
    file->write("***\n");
    file->write("Elemental Profiles\n");
    
    // Write each sample group
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != end(); it++)
    {
        file->write("**\n");
        file->write(QString::fromStdString(it->first + "\n").toUtf8());
        it->second.writetofile(file);
    }
    
    return true;
}

bool SourceSinkData::WriteToFile(QFile* file)
{
    return WriteDataToFile(file);
}

bool SourceSinkData::ReadFromFile(QFile* fil)
{
    // Clear existing data
    Clear();
    
    // Parse JSON from file
    QJsonObject jsondoc = QJsonDocument().fromJson(fil->readAll()).object();
    
    // Load all components
    ReadElementDatafromJsonObject(jsondoc["Element Data"].toObject());
    ReadElementInformationfromJsonObject(jsondoc["Element Information"].toObject());
    ReadToolsUsedFromJsonObject(jsondoc["Tools used"].toArray());
    ReadOptionsfromJsonObject(jsondoc["Options"].toObject());
    target_group_ = jsondoc["Target Group"].toString().toStdString();
    
    return true;
}

QJsonObject SourceSinkData::ElementInformationToJsonObject()
{
    QJsonObject json_object;

    // Serialize metadata for each element
    for (map<string, element_information>::iterator it = element_information_.begin(); it != element_information_.end(); it++)
    {
        QJsonObject elem_info_json_obj;
        elem_info_json_obj["Role"] = Role(it->second.Role);
        elem_info_json_obj["Standard Ratio"] = it->second.standard_ratio;
        elem_info_json_obj["Base Element"] = QString::fromStdString(it->second.base_element);
        elem_info_json_obj["Include"] = it->second.include_in_analysis;

        json_object[QString::fromStdString(it->first)] = elem_info_json_obj;
    }

    return json_object;
}

QString SourceSinkData::Role(const element_information::role& role)
{
    // Convert enum to string representation
    if (role == element_information::role::do_not_include)
        return "DoNotInclude";
    else if (role == element_information::role::element)
        return "Element";
    else if (role == element_information::role::isotope)
        return "Isotope";
    else if (role == element_information::role::particle_size)
        return "ParticleSize";
    else if (role == element_information::role::organic_carbon)
        return "OM";

    // Default for unrecognized values
    return "DoNotInclude";
}

element_information::role SourceSinkData::Role(const QString& role_string)
{
    // Convert string to enum representation
    if (role_string == "DoNotInclude")
        return element_information::role::do_not_include;
    else if (role_string == "Element")
        return element_information::role::element;
    else if (role_string == "Isotope")
        return element_information::role::isotope;
    else if (role_string == "ParticleSize")
        return element_information::role::particle_size;
    else if (role_string == "OM")
        return element_information::role::organic_carbon;

    // Default for unrecognized values
    return element_information::role::do_not_include;
}

bool SourceSinkData::PerformRegressionVsOMAndSize(
    const string& om,
    const string& particle_size,
    regression_form form,
    const double& p_value_threshold)
{
    // Store OM and size constituent names for later use in corrections
    omconstituent_ = om;
    sizeconsituent_ = particle_size;
    regression_p_value_threshold_ = p_value_threshold;

    // Compute regression models for all sample groups
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != end(); it++)
    {
        it->second.SetRegressionModels(om, particle_size, form, p_value_threshold);
    }

    return true;
}

void SourceSinkData::OutlierAnalysisForAll(const double& lower_threshold, const double& upper_threshold)
{
    // Perform outlier detection on each source group
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != end(); it++)
    {
        // Skip target group - only analyze sources
        if (it->first != target_group_)
        {
            it->second.DetectOutliers(lower_threshold, upper_threshold);
        }
    }
}

DFA_result SourceSinkData::DiscriminantFunctionAnalysis()
{
    DFA_result result;

    // Initialize result vectors for all sources
    const size_t num_sources = size() - 1;  // Exclude target
    result.F_test_P_value = CMBVector(num_sources);
    result.p_values = CMBVector(num_sources);
    result.wilkslambda = CMBVector(num_sources);

    // Perform DFA for each source vs all others
    size_t counter = 0;
    for (map<string, Elemental_Profile_Set>::iterator source = begin(); source != end(); source++)
    {
        // Skip target group
        if (source->first != target_group_)
        {
            // Run one-vs-rest DFA for this source
            DFA_result source_result = DiscriminantFunctionAnalysis(source->first);

            // Extract and store results
            result.F_test_P_value[counter] = source_result.F_test_P_value[0];
            result.F_test_P_value.SetLabel(counter, source->first);

            result.p_values[counter] = source_result.p_values[0];
            result.p_values.SetLabel(counter, source->first);

            result.wilkslambda[counter] = source_result.wilkslambda[0];
            result.wilkslambda.SetLabel(counter, source->first);

            result.eigen_vectors.Append(source->first, source_result.eigen_vectors[source->first + " vs the rest"]);
            result.multi_projected.Append(source->first, source_result.projected);

            counter++;
        }
    }

    return result;
}

DFA_result SourceSinkData::DiscriminantFunctionAnalysis(const string& source1, const string& source2)
{
    // Create temporary dataset with only two sources
    SourceSinkData two_sources;
    two_sources.AppendSampleSet(source1, at(source1));
    two_sources.AppendSampleSet(source2, at(source2));
    two_sources.PopulateElementInformation(&element_information_);

    DFA_result result;

    // Compute discriminant eigenvector
    CMBVector eigen_vector = two_sources.DFA_eigvector();

    // Return empty result if eigenvector computation failed
    if (eigen_vector.size() == 0)
    {
        return result;
    }

    // Project samples onto discriminant axis
    result.projected = two_sources.DFA_Projected(source1, source2);

    // Compute F-test p-value for separation
    result.F_test_P_value = CMBVector(1);
    result.F_test_P_value[0] = result.projected.FTest_p_value();

    // Store eigenvector
    result.eigen_vectors.Append(source1 + " vs " + source2, eigen_vector);

    // Compute discriminant p-value and Wilks' Lambda
    double p_value = two_sources.DFA_P_Value();
    result.p_values = CMBVector(1);
    result.p_values[0] = p_value;

    result.wilkslambda = CMBVector(1);
    result.wilkslambda[0] = two_sources.WilksLambda();

    return result;
}

DFA_result SourceSinkData::DiscriminantFunctionAnalysis(const string& source1)
{
    // Create temporary dataset with one source vs all others
    SourceSinkData two_sources;
    two_sources.AppendSampleSet(source1, at(source1));
    two_sources.AppendSampleSet("Others", TheRest(source1));
    two_sources.PopulateElementInformation(&element_information_);

    DFA_result result;

    // Compute discriminant eigenvector
    CMBVector eigen_vector = two_sources.DFA_eigvector();

    // Project all samples onto discriminant axis
    result.projected = two_sources.DFA_Projected(source1, this);

    // Project pairwise comparison (source vs others)
    CMBVectorSet pairwise_projected = two_sources.DFA_Projected(source1, "Others");

    // Compute F-test p-value from pairwise projection
    result.F_test_P_value = CMBVector(1);
    result.F_test_P_value[0] = pairwise_projected.FTest_p_value();

    // Store eigenvector with descriptive label
    result.eigen_vectors.Append(source1 + " vs the rest", eigen_vector);

    // Compute discriminant p-value and Wilks' Lambda
    double p_value = two_sources.DFA_P_Value();
    result.p_values = CMBVector(1);
    result.p_values[0] = p_value;

    result.wilkslambda = CMBVector(1);
    result.wilkslambda[0] = two_sources.WilksLambda();

    return result;
}
int SourceSinkData::TotalNumberofSourceSamples() const
{
    int count = 0;

    // Sum samples across all source groups
    for (map<string, Elemental_Profile_Set>::const_iterator source_group = cbegin(); source_group != cend(); source_group++)
    {
        // Skip target group - only count source samples
        if (source_group->first != target_group_)
        {
            count += source_group->second.size();
        }
    }

    return count;
}

CMBVector SourceSinkData::DFATransformed(const CMBVector& eigenvector, const string& source_group)
{
    // Initialize result vector for discriminant scores
    CMBVector discriminant_scores(operator[](source_group).size());

    // Project each sample onto the discriminant axis
    size_t sample_index = 0;
    for (map<string, Elemental_Profile>::iterator sample = operator[](source_group).begin();
        sample != operator[](source_group).end();
        sample++)
    {
        // Compute dot product: discriminant score = eigenvector · sample_profile
        discriminant_scores[sample_index] = sample->second.CalculateDotProduct(eigenvector);
        discriminant_scores.SetLabel(sample_index, sample->first);

        sample_index++;
    }

    return discriminant_scores;
}

Elemental_Profile_Set SourceSinkData::TheRest(const string& excluded_source)
{
    Elemental_Profile_Set combined_sources;

    // Iterate through all sample groups
    for (map<string, Elemental_Profile_Set>::iterator profile_set = begin();
        profile_set != end();
        profile_set++)
    {
        // Skip the excluded source and target group
        if (profile_set->first != excluded_source && profile_set->first != target_group_)
        {
            // Add all samples from this source group
            for (map<string, Elemental_Profile>::iterator profile = profile_set->second.begin();
                profile != profile_set->second.end();
                profile++)
            {
                combined_sources.AppendProfile(profile->first, profile->second);
            }
        }
    }

    return combined_sources;
}


CMBVector SourceSinkData::BracketTest(const string& target_sample, bool correct_based_on_om_n_size)
{
    vector<string> element_names = GetElementNames();
    const size_t num_elements = element_names.size();

    // Initialize result vectors
    CMBVector bracket_test_results(num_elements);
    CVector exceeds_max(num_elements);  // Flags for concentrations above source maximum
    CVector below_min(num_elements);    // Flags for concentrations below source minimum

    exceeds_max.SetAllValues(1.0);  // Assume all exceed until proven otherwise
    below_min.SetAllValues(1.0);    // Assume all below until proven otherwise

    // Create corrected dataset for testing
    SourceSinkData corrected_data = CreateCorrectedAndFilteredDataset(
        false, false, correct_based_on_om_n_size, target_sample);

    // Check each source group's concentration ranges
    for (map<string, Elemental_Profile_Set>::iterator it = corrected_data.begin();
        it != corrected_data.end();
        it++)
    {
        // Skip target group
        if (it->first != target_group_)
        {
            // Check each element
            for (size_t i = 0; i < num_elements; i++)
            {
                bracket_test_results.SetLabel(i, element_names[i]);

                double target_concentration = corrected_data.at(target_group_)
                    .GetProfile(target_sample)->at(element_names[i]);
                double source_maximum = it->second.GetElementDistribution(element_names[i])
                    ->GetMaximum();
                double source_minimum = it->second.GetElementDistribution(element_names[i])
                    ->GetMinimum();

                // Check if target is within this source's range
                if (target_concentration <= source_maximum)
                {
                    exceeds_max[i] = 0;  // Target does not exceed max
                }
                if (target_concentration >= source_minimum)
                {
                    below_min[i] = 0;    // Target is not below min
                }
            }
        }
    }

    // Compile results and annotate failures
    for (size_t i = 0; i < num_elements; i++)
    {
        // Fail if either above all maxima or below all minima
        bracket_test_results[i] = max(exceeds_max[i], below_min[i]);

        // Annotate failures in target sample notes
        if (exceeds_max[i] > 0.5)
        {
            at(target_group_).GetProfile(target_sample)->AppendtoNotes(
                element_names[i] + " value is higher than the maximum of the sources");
        }
        else if (below_min[i] > 0.5)
        {
            at(target_group_).GetProfile(target_sample)->AppendtoNotes(
                element_names[i] + " value is lower than the minimum of the sources");
        }
    }

    return bracket_test_results;
}


CMBMatrix SourceSinkData::BracketTest(
    bool correct_based_on_om_n_size,
    bool exclude_elements,
    bool exclude_samples)
{
    // Initialize result matrix
    // Note: Matrix is constructed as (num_rows, num_cols) but accessed as [row][col]
    const size_t num_target_samples = at(target_group_).size();
    const size_t num_elements = CountElements(exclude_elements);
    CMBMatrix bracket_results(num_target_samples, num_elements);

    // Test each target sample
    size_t sample_index = 0;
    for (map<string, Elemental_Profile>::iterator sample = at(target_group_).begin();
        sample != at(target_group_).end();
        sample++)
    {
        // Prepare dataset for this target sample
        SourceSinkData corrected_data;
        if (correct_based_on_om_n_size)
        {
            corrected_data = CreateCorrectedAndFilteredDataset(
                exclude_samples, exclude_elements, true, sample->first);
        }
        else
        {
            corrected_data = CreateCorrectedAndFilteredDataset(
                exclude_samples, exclude_elements, false);
        }

        // Perform bracket test for this sample
        vector<string> element_names = corrected_data.GetElementNames();
        CMBVector bracket_vector = corrected_data.BracketTest(sample->first, false);

        // Store results in matrix
        for (size_t element_index = 0; element_index < element_names.size(); element_index++)
        {
            bracket_results[element_index][sample_index] = bracket_vector.valueAt(element_index);
            bracket_results.SetRowLabel(element_index, element_names[element_index]);
        }
        bracket_results.SetColumnLabel(sample_index, sample->first);

        sample_index++;
    }

    return bracket_results;
}



SourceSinkData SourceSinkData::BoxCoxTransformed(bool calculate_optimal_lambda)
{
    // Compute optimal lambda parameters for each element
    CMBVector lambda_parameters = OptimalBoxCoxParameters();

    // Create copy of data for transformation
    SourceSinkData transformed_data(*this);

    // Apply Box-Cox transformation to each source group
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != end(); it++)
    {
        // Skip target group - only transform sources
        if (it->first != target_group_)
        {
            if (calculate_optimal_lambda)
            {
                // Transform using optimal lambda parameters
                transformed_data[it->first] = it->second.ApplyBoxCoxTransform(&lambda_parameters);
            }
            else
            {
                // Transform using default parameters
                transformed_data[it->first] = it->second.ApplyBoxCoxTransform();
            }
        }
    }

    // Recalculate distributions for transformed data
    transformed_data.PopulateElementDistributions();
    transformed_data.AssignAllDistributions();

    return transformed_data;
}

map<string, ConcentrationSet> SourceSinkData::ExtractConcentrationSet()
{
    map<string, ConcentrationSet> element_concentrations;
    vector<string> element_names = GetElementNames();

    // Build concentration set for each element
    for (size_t i = 0; i < element_names.size(); i++)
    {
        ConcentrationSet concentration_set;

        // Collect concentrations from all source groups
        for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != end(); it++)
        {
            // Skip target group - only collect from sources
            if (it->first != target_group_)
            {
                // Add each sample's concentration for this element
                for (map<string, Elemental_Profile>::iterator sample = it->second.begin();
                    sample != it->second.end();
                    sample++)
                {
                    concentration_set.AppendValue(sample->second[element_names[i]]);
                }
            }
        }

        element_concentrations[element_names[i]] = concentration_set;
    }

    return element_concentrations;
}

CMBVector SourceSinkData::OptimalBoxCoxParameters()
{
    // Extract concentration distributions for all elements
    map<string, ConcentrationSet> concentration_sets = ExtractConcentrationSet();
    vector<string> element_names = GetElementNames();

    // Compute optimal lambda for each element
    CMBVector lambda_parameters(element_names.size());

    for (size_t i = 0; i < element_names.size(); i++)
    {
        // Find optimal lambda using maximum likelihood estimation
        // Search range: [-5, 5], refinement iterations: 10
        lambda_parameters[i] = concentration_sets[element_names[i]].FindOptimalBoxCoxParameter(-5, 5, 10);
    }

    // Label parameters with element names
    lambda_parameters.SetLabels(element_names);

    return lambda_parameters;
}
Elemental_Profile SourceSinkData::t_TestPValue(const string& source1, const string& source2, bool use_log)
{
    vector<string> element_names = GetElementNames();
    Elemental_Profile p_values;

    // Perform t-test for each element
    for (size_t i = 0; i < element_names.size(); i++)
    {
        // Get concentration distributions for both sources
        ConcentrationSet concentration_set1 = *at(source1).GetElementDistribution(element_names[i]);
        ConcentrationSet concentration_set2 = *at(source2).GetElementDistribution(element_names[i]);

        // Compute statistics in appropriate space
        double std1, std2, mean1, mean2;
        if (!use_log)
        {
            // Linear-space statistics
            std1 = concentration_set1.CalculateStdDev();
            std2 = concentration_set2.CalculateStdDev();
            mean1 = concentration_set1.CalculateMean();
            mean2 = concentration_set2.CalculateMean();
        }
        else
        {
            // Log-space statistics
            std1 = concentration_set1.CalculateStdDevLog();
            std2 = concentration_set2.CalculateStdDevLog();
            mean1 = concentration_set1.CalculateMeanLog();
            mean2 = concentration_set2.CalculateMeanLog();
        }

        // Compute t-statistic
        double standard_error = sqrt(pow(std1, 2) / concentration_set1.size() +
            pow(std2, 2) / concentration_set2.size());
        double t_statistic = (mean1 - mean2) / standard_error;

        // Compute two-tailed p-value
        size_t degrees_of_freedom = concentration_set1.size() + concentration_set2.size() - 2;
        double p_value_upper = gsl_cdf_tdist_Q(t_statistic, degrees_of_freedom);
        double p_value_lower = gsl_cdf_tdist_P(t_statistic, degrees_of_freedom);

        // Two-tailed: use minimum of tails, then sum
        p_value_upper = min(p_value_upper, 1.0 - p_value_upper);
        p_value_lower = min(p_value_lower, 1.0 - p_value_lower);
        double p_value = p_value_upper + p_value_lower;

        p_values.AppendElement(element_names[i], p_value);
    }

    return p_values;
}

Elemental_Profile SourceSinkData::DifferentiationPower(const string& source1, const string& source2, bool use_log)
{
    vector<string> element_names = GetElementNames();
    Elemental_Profile differentiation_powers;

    // Compute differentiation power for each element
    for (size_t i = 0; i < element_names.size(); i++)
    {
        // Get concentration distributions for both sources
        ConcentrationSet concentration_set1 = *at(source1).GetElementDistribution(element_names[i]);
        ConcentrationSet concentration_set2 = *at(source2).GetElementDistribution(element_names[i]);

        // Compute statistics in appropriate space
        double std1, std2, mean1, mean2;
        if (!use_log)
        {
            // Linear-space statistics
            std1 = concentration_set1.CalculateStdDev();
            std2 = concentration_set2.CalculateStdDev();
            mean1 = concentration_set1.CalculateMean();
            mean2 = concentration_set2.CalculateMean();
        }
        else
        {
            // Log-space statistics
            std1 = concentration_set1.CalculateStdDevLog();
            std2 = concentration_set2.CalculateStdDevLog();
            mean1 = concentration_set1.CalculateMeanLog();
            mean2 = concentration_set2.CalculateMeanLog();
        }

        // Compute differentiation power: 2 × |Δμ| / (σ₁ + σ₂)
        double diff_power = 2.0 * fabs(mean1 - mean2) / (std1 + std2);

        differentiation_powers.AppendElement(element_names[i], diff_power);
    }

    return differentiation_powers;
}

Elemental_Profile_Set SourceSinkData::DifferentiationPower(bool use_log, bool include_target)
{
    Elemental_Profile_Set all_pairs;

    // Compare all pairs of source groups
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != prev(end()); it++)
    {
        if (include_target || it->first != target_group_)
        {
            for (map<string, Elemental_Profile_Set>::iterator it2 = next(it); it2 != end(); it2++)
            {
                if (include_target || it2->first != target_group_)
                {
                    Elemental_Profile diff_power = DifferentiationPower(it->first, it2->first, use_log);
                    all_pairs.AppendProfile(it->first + " and " + it2->first, diff_power);
                }
            }
        }
    }

    return all_pairs;
}

Elemental_Profile SourceSinkData::DifferentiationPower_Percentage(const string& source1, const string& source2)
{
    vector<string> element_names = GetElementNames();
    Elemental_Profile classification_percentages;

    // Compute classification percentage for each element
    for (size_t i = 0; i < element_names.size(); i++)
    {
        // Get concentration distributions
        ConcentrationSet concentration_set1 = *at(source1).GetElementDistribution(element_names[i]);
        ConcentrationSet concentration_set2 = *at(source2).GetElementDistribution(element_names[i]);

        // Pool all samples and compute ranks
        ConcentrationSet combined_set = concentration_set1;
        combined_set.AppendSet(concentration_set2);
        vector<unsigned int> ranks = combined_set.CalculateRanks();

        // Count correct classifications
        size_t n1 = concentration_set1.size();
        int set1_below_limit = aquiutils::CountLessThan(ranks, n1, n1, false);
        int set1_above_limit = aquiutils::CountGreaterThan(ranks, n1, n1, false);
        int set2_below_limit = aquiutils::CountLessThan(ranks, n1, n1, true);
        int set2_above_limit = aquiutils::CountGreaterThan(ranks, n1, n1, true);

        // Compute classification success rates
        double set1_fraction = double(set1_below_limit + set2_above_limit) / double(combined_set.size());
        double set2_fraction = double(set1_above_limit + set2_below_limit) / double(combined_set.size());

        // Use best classification direction
        double classification_percentage = max(set1_fraction, set2_fraction);

        classification_percentages.AppendElement(element_names[i], classification_percentage);
    }

    return classification_percentages;
}

Elemental_Profile_Set SourceSinkData::DifferentiationPower_Percentage(bool include_target)
{
    Elemental_Profile_Set all_pairs;

    // Compare all pairs of source groups
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != prev(end()); it++)
    {
        if (include_target || it->first != target_group_)
        {
            for (map<string, Elemental_Profile_Set>::iterator it2 = next(it); it2 != end(); it2++)
            {
                if (include_target || it2->first != target_group_)
                {
                    Elemental_Profile diff_power_pct = DifferentiationPower_Percentage(it->first, it2->first);
                    all_pairs.AppendProfile(it->first + " and " + it2->first, diff_power_pct);
                }
            }
        }
    }

    return all_pairs;
}

Elemental_Profile_Set SourceSinkData::DifferentiationPower_P_value(bool include_target)
{
    Elemental_Profile_Set all_pairs;

    // Compare all pairs of source groups
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != prev(end()); it++)
    {
        if (include_target || it->first != target_group_)
        {
            for (map<string, Elemental_Profile_Set>::iterator it2 = next(it); it2 != end(); it2++)
            {
                if (include_target || it2->first != target_group_)
                {
                    Elemental_Profile p_values = t_TestPValue(it->first, it2->first, false);
                    all_pairs.AppendProfile(it->first + " and " + it2->first, p_values);
                }
            }
        }
    }

    return all_pairs;
}

vector<string> SourceSinkData::NegativeValueCheck()
{
    vector<string> error_messages;

    // Ensure element ordering is current
    PopulateConstituentOrders();

    // Check each source group for negative values
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != prev(end()); it++)
    {
        vector<string> negative_elements = it->second.CheckForNegativeValues(element_order_);

        // Generate error message for each problematic element
        for (size_t i = 0; i < negative_elements.size(); i++)
        {
            error_messages.push_back("There are zero or negative values for element '" +
                negative_elements[i] + "' in sample group '" +
                it->first + "'");
        }
    }

    return error_messages;
}

void SourceSinkData::IncludeExcludeAllElements(bool include_in_analysis)
{
    // Set inclusion flag for all elements
    for (map<string, element_information>::iterator it = element_information_.begin();
        it != element_information_.end();
        it++)
    {
        it->second.include_in_analysis = include_in_analysis;
    }
}

double SourceSinkData::GrandMean(const string& element, bool use_log)
{
    double weighted_sum = 0.0;
    double total_count = 0.0;

    // Accumulate weighted means from each source group
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != prev(end()); it++)
    {
        // Skip target group
        if (it->first != target_group_)
        {
            ConcentrationSet* element_dist = it->second.GetElementDistribution(element);
            size_t sample_count = element_dist->size();

            if (!use_log)
            {
                // Linear-space mean
                weighted_sum += element_dist->CalculateMean() * sample_count;
            }
            else
            {
                // Log-space mean
                weighted_sum += element_dist->CalculateMeanLog() * sample_count;
            }

            total_count += sample_count;
        }
    }

    // Compute weighted average
    return weighted_sum / total_count;
}

Elemental_Profile_Set SourceSinkData::LumpAllProfileSets()
{
    Elemental_Profile_Set combined_sources;

    // Pool samples from all source groups
    for (map<string, Elemental_Profile_Set>::const_iterator it = begin(); it != prev(end()); it++)
    {
        // Skip target group
        if (it->first != target_group_)
        {
            combined_sources.AppendProfiles(it->second, nullptr);
        }
    }

    // Recalculate distributions for combined dataset
    combined_sources.UpdateElementDistributions();

    return combined_sources;
}

CMBVector SourceSinkData::ANOVA(bool use_log)
{
    vector<string> element_names = GetElementNames();
    CMBVector p_values(element_names.size());
    p_values.SetLabels(element_names);

    // Perform ANOVA for each element
    for (size_t i = 0; i < element_names.size(); i++)
    {
        ANOVA_info anova_result = ANOVA(element_names[i], use_log);
        p_values[i] = anova_result.p_value;
    }

    return p_values;
}

ANOVA_info SourceSinkData::ANOVA(const string& element, bool use_log)
{
    ANOVA_info anova;

    // Combine all source samples for total variance calculation
    Elemental_Profile_Set all_sources = LumpAllProfileSets();
    ConcentrationSet* all_element_data = all_sources.GetElementDistribution(element);

    if (!use_log)
    {
        // Linear-space ANOVA

        // Total sum of squares (SST)
        anova.SST = all_element_data->CalculateSSE();

        // Between-group sum of squares (SSB) and within-group sum of squares (SSW)
        double grand_mean = all_element_data->CalculateMean();
        double sum_between = 0.0;
        double sum_within = 0.0;

        for (map<string, Elemental_Profile_Set>::iterator source_group = begin();
            source_group != end();
            source_group++)
        {
            if (source_group->first != target_group_)
            {
                ConcentrationSet* group_data = source_group->second.GetElementDistribution(element);
                double group_mean = group_data->CalculateMean();
                size_t group_size = group_data->size();

                // SSB: variance of group means weighted by sample size
                sum_between += pow(group_mean - grand_mean, 2) * group_size;

                // SSW: sum of within-group variances
                sum_within += group_data->CalculateSSE();
            }
        }

        anova.SSB = sum_between;
        anova.SSW = sum_within;
    }
    else
    {
        // Log-space ANOVA

        // Total sum of squares (SST)
        double total_std_log = all_element_data->CalculateStdDevLog();
        size_t total_size = all_element_data->size();
        anova.SST = pow(total_std_log, 2) * (total_size - 1);

        // Between-group sum of squares (SSB) and within-group sum of squares (SSW)
        double grand_mean_log = all_element_data->CalculateMeanLog();
        double sum_between = 0.0;
        double sum_within = 0.0;

        for (map<string, Elemental_Profile_Set>::iterator source_group = begin();
            source_group != end();
            source_group++)
        {
            if (source_group->first != target_group_)
            {
                ConcentrationSet* group_data = source_group->second.GetElementDistribution(element);
                double group_mean_log = group_data->CalculateMeanLog();
                size_t group_size = group_data->size();

                // SSB: variance of group means weighted by sample size
                sum_between += pow(group_mean_log - grand_mean_log, 2) * group_size;

                // SSW: sum of within-group variances
                sum_within += group_data->CalculateSSELog();
            }
        }

        anova.SSB = sum_between;
        anova.SSW = sum_within;
    }

    // Compute mean squares and F-statistic
    size_t num_groups = this->size() - 1;  // Exclude target
    size_t total_samples = all_element_data->size();

    // Degrees of freedom
    size_t df_between = num_groups - 1;
    size_t df_within = total_samples - num_groups;

    // Mean squares
    anova.MSB = anova.SSB / double(df_between);
    anova.MSW = anova.SSW / double(df_within);

    // F-statistic
    anova.F = anova.MSB / anova.MSW;

    // P-value from F-distribution
    anova.p_value = gsl_cdf_fdist_Q(anova.F, df_between, total_samples);

    return anova;
}
void SourceSinkData::IncludeExcludeElementsBasedOn(const vector<string>& elements)
{
    // First, exclude all elements
    for (map<string, element_information>::iterator element = element_information_.begin();
        element != element_information_.end();
        element++)
    {
        element->second.include_in_analysis = false;
    }

    // Then, include only specified elements
    for (size_t i = 0; i < elements.size(); i++)
    {
        if (element_information_.count(elements[i]) > 0)
        {
            element_information_[elements[i]].include_in_analysis = true;
        }
    }
}

SourceSinkData SourceSinkData::RandomlyEliminateSourceSamples(const double& percentage)
{
    SourceSinkData reduced_dataset;

    // Randomly select samples to eliminate
    vector<string> samples_to_eliminate = RandomlypickSamples(percentage / 100.0);

    // Copy groups with specified samples eliminated
    for (map<string, Elemental_Profile_Set>::const_iterator it = cbegin(); it != cend(); it++)
    {
        if (it->first != target_group_)
        {
            // Remove randomly selected samples from source groups
            reduced_dataset[it->first] = it->second.EliminateSamples(
                samples_to_eliminate, &element_information_);
        }
        else
        {
            // Preserve target group unchanged
            reduced_dataset[it->first] = it->second;
        }
    }

    // Copy metadata and settings
    reduced_dataset.omconstituent_ = omconstituent_;
    reduced_dataset.sizeconsituent_ = sizeconsituent_;
    reduced_dataset.target_group_ = target_group_;

    // Recalculate distributions for reduced dataset
    reduced_dataset.PopulateElementInformation(&element_information_);
    reduced_dataset.PopulateElementDistributions();
    reduced_dataset.AssignAllDistributions();

    return reduced_dataset;
}

Elemental_Profile SourceSinkData::Sample(const string& sample_name) const
{
    // Search all groups for the specified sample
    for (map<string, Elemental_Profile_Set>::const_iterator it = cbegin(); it != cend(); it++)
    {
        if (it->second.count(sample_name) == 1)
        {
            return it->second.at(sample_name);
        }
    }

    // Sample not found - return empty profile
    return Elemental_Profile();
}
SourceSinkData SourceSinkData::ReplaceSourceAsTarget(const string& source_sample_name) const
{
    // Create copy of current dataset
    SourceSinkData modified_dataset = *this;

    // Extract specified source sample
    Elemental_Profile target_profile = Sample(source_sample_name);

    // Create new target group with this sample
    Elemental_Profile_Set new_target_group = Elemental_Profile_Set();
    new_target_group.AppendProfile(source_sample_name, target_profile);
    modified_dataset[target_group_] = new_target_group;

    // Copy metadata and settings
    modified_dataset.omconstituent_ = omconstituent_;
    modified_dataset.sizeconsituent_ = sizeconsituent_;
    modified_dataset.target_group_ = target_group_;

    // Recalculate distributions
    modified_dataset.PopulateElementInformation(&element_information_);
    modified_dataset.PopulateElementDistributions();
    modified_dataset.AssignAllDistributions();

    return modified_dataset;
}

CMBTimeSeriesSet SourceSinkData::BootStrap(
    const double& percentage,
    unsigned int num_iterations,
    string target_sample,
    bool use_softmax)
{
    // Initialize result time series
    CMBTimeSeriesSet contribution_time_series(numberofsourcesamplesets_);

    // Set source names for time series
    for (size_t source_idx = 0; source_idx < numberofsourcesamplesets_; source_idx++)
    {
        contribution_time_series.setname(source_idx, samplesetsorder_[source_idx]);
    }

    // Perform bootstrap iterations
    for (size_t iteration = 0; iteration < num_iterations; iteration++)
    {
        // Create bootstrapped dataset with randomly excluded samples
        SourceSinkData bootstrapped_data = RandomlyEliminateSourceSamples(percentage);
        bootstrapped_data.InitializeParametersAndObservations(target_sample);

        // Update progress if progress tracker available
        if (rtw_)
        {
            rtw_->SetProgress(double(iteration) / double(num_iterations));
        }

        // Solve CMB model with appropriate transformation
        if (use_softmax)
        {
            bootstrapped_data.SolveLevenberg_Marquardt(transformation::softmax);
        }
        else
        {
            bootstrapped_data.SolveLevenberg_Marquardt(transformation::linear);
        }

        // Record contributions from this iteration
        contribution_time_series.append(iteration, bootstrapped_data.GetContributionVector().vec);
    }

    return contribution_time_series;
}

bool SourceSinkData::BootStrap(
    Results* results,
    const double& percentage,
    unsigned int num_iterations,
    string target_sample,
    bool use_softmax)
{
    // Initialize contribution time series
    CMBTimeSeriesSet* contributions = new CMBTimeSeriesSet(numberofsourcesamplesets_);

    // Set source names
    for (size_t source_idx = 0; source_idx < numberofsourcesamplesets_; source_idx++)
    {
        contributions->setname(source_idx, samplesetsorder_[source_idx]);
    }

    // Perform bootstrap iterations
    for (size_t iteration = 0; iteration < num_iterations; iteration++)
    {
        // Create bootstrapped dataset
        SourceSinkData bootstrapped_data = RandomlyEliminateSourceSamples(percentage);
        bootstrapped_data.InitializeParametersAndObservations(target_sample);

        // Update progress
        if (rtw_)
        {
            rtw_->SetProgress(double(iteration) / double(num_iterations));
        }

        // Solve CMB model
        if (use_softmax)
        {
            bootstrapped_data.SolveLevenberg_Marquardt(transformation::softmax);
        }
        else
        {
            bootstrapped_data.SolveLevenberg_Marquardt(transformation::linear);
        }

        // Record contributions
        contributions->append(iteration, bootstrapped_data.GetContributionVector().vec);
    }

    // ========== Result 1: Contribution Time Series ==========
    ResultItem contributions_result;
    results->SetName("Error Analysis for target sample '" + target_sample + "'");
    contributions_result.SetName("Error Analysis");
    contributions_result.SetResult(contributions);
    contributions_result.SetType(result_type::stacked_bar_chart);
    contributions_result.SetShowAsString(false);
    contributions_result.SetShowTable(true);

    // Only show graph for small sample sizes (to avoid clutter)
    if (num_iterations < 101)
    {
        contributions_result.SetShowGraph(true);
    }
    else
    {
        contributions_result.SetShowGraph(false);
    }

    contributions_result.SetYLimit(_range::high, 1);
    contributions_result.SetYLimit(_range::low, 0);
    contributions_result.SetXAxisMode(xaxis_mode::counter);
    contributions_result.setYAxisTitle("Contribution");
    contributions_result.setXAxisTitle("Sample");
    results->Append(contributions_result);

    // ========== Result 2: Posterior Distributions ==========
    CMBTimeSeriesSet* distributions = new CMBTimeSeriesSet();
    *distributions = contributions->distribution(100, 0, 0);

    ResultItem distributions_result;
    distributions_result.SetName("Posterior Distributions");
    distributions_result.SetShowAsString(false);
    distributions_result.SetShowTable(true);
    distributions_result.SetType(result_type::distribution);
    distributions_result.SetResult(distributions);
    results->Append(distributions_result);

    // ========== Result 3: 95% Credible Intervals ==========
    RangeSet* credible_intervals = new RangeSet();

    for (size_t i = 0; i < GetSourceOrder().size(); i++)
    {
        Range interval;

        // Compute statistics for this source
        double percentile_2_5 = contributions->at(i).percentile(0.025);
        double percentile_97_5 = contributions->at(i).percentile(0.975);
        double mean_contribution = contributions->at(i).mean();
        double median_contribution = contributions->at(i).percentile(0.5);

        // Configure credible interval
        interval.Set(_range::low, percentile_2_5);
        interval.Set(_range::high, percentile_97_5);
        interval.SetMean(mean_contribution);
        interval.SetMedian(median_contribution);

        (*credible_intervals)[contributions->getSeriesName(i)] = interval;
    }

    ResultItem intervals_result;
    intervals_result.SetName("Source Contribution Credible Intervals");
    intervals_result.SetShowAsString(true);
    intervals_result.SetShowTable(true);
    intervals_result.SetType(result_type::rangeset);
    intervals_result.SetResult(credible_intervals);
    intervals_result.SetYAxisMode(yaxis_mode::normal);
    intervals_result.SetYLimit(_range::high, 1.0);
    intervals_result.SetYLimit(_range::low, 0);
    results->Append(intervals_result);

    return true;
}

CMBTimeSeriesSet SourceSinkData::VerifySource(
    const string& source_group,
    bool use_softmax,
    bool apply_om_size_correction)
{
    // Initialize with first target sample (will be replaced in loop)
    InitializeParametersAndObservations(GetSampleSet(target_group_)->begin()->first);

    // Initialize result time series
    CMBTimeSeriesSet validation_results(numberofsourcesamplesets_);

    // Set source names
    for (size_t source_idx = 0; source_idx < numberofsourcesamplesets_; source_idx++)
    {
        validation_results.setname(source_idx, samplesetsorder_[source_idx]);
    }

    // Perform leave-one-out validation for each sample in the source group
    size_t valid_sample_count = 0;
    for (map<string, Elemental_Profile>::iterator sample = at(source_group).begin();
        sample != at(source_group).end();
        sample++)
    {
        // Create dataset with this sample as target
        SourceSinkData test_dataset = ReplaceSourceAsTarget(sample->first);

        // Apply corrections if requested
        SourceSinkData corrected_dataset = test_dataset.CreateCorrectedDataset(
            sample->first, apply_om_size_correction, test_dataset.GetElementInformation());
        corrected_dataset.SetProgressWindow(rtw_);

        // Check for negative values (would cause problems in CMB)
        vector<string> negative_elements = corrected_dataset.NegativeValueCheck();

        if (negative_elements.size() == 0)
        {
            // No negative values - proceed with CMB solution
            test_dataset.InitializeParametersAndObservations(sample->first);

            // Update progress
            if (rtw_)
            {
                rtw_->SetProgress(double(valid_sample_count) / double(at(source_group).size()));
            }

            // Solve CMB model
            if (use_softmax)
            {
                test_dataset.SolveLevenberg_Marquardt(transformation::softmax);
            }
            else
            {
                test_dataset.SolveLevenberg_Marquardt(transformation::linear);
            }

            // Record contributions and label with sample name
            validation_results.append(valid_sample_count, test_dataset.GetContributionVector().vec);
            validation_results.SetLabel(valid_sample_count, sample->first);
            valid_sample_count++;
        }
        else
        {
            // Negative values detected - skip this sample and report
            for (size_t i = 0; i < negative_elements.size(); i++)
            {
                qDebug() << QString::fromStdString(negative_elements[i]);
            }
        }
    }

    return validation_results;
}

CMBTimeSeriesSet SourceSinkData::LM_Batch(
    transformation transform,
    bool apply_om_size_correction,
    map<string, vector<string>>& negative_elements)
{
    // Initialize with first target sample (will be replaced in loop)
    InitializeParametersAndObservations(GetSampleSet(target_group_)->begin()->first);

    // Initialize result time series
    CMBTimeSeriesSet contribution_results(numberofsourcesamplesets_);

    // Set source names
    for (size_t source_idx = 0; source_idx < numberofsourcesamplesets_; source_idx++)
    {
        contribution_results.setname(source_idx, samplesetsorder_[source_idx]);
    }

    // Solve CMB for each target sample
    size_t valid_sample_count = 0;
    for (map<string, Elemental_Profile>::iterator sample = at(target_group_).begin();
        sample != at(target_group_).end();
        sample++)
    {
        // Skip samples with empty names
        if (sample->first != "")
        {
            // Apply corrections if requested
            SourceSinkData corrected_dataset = CreateCorrectedDataset(
                sample->first, apply_om_size_correction, GetElementInformation());

            // Check for negative values
            negative_elements[sample->first] = corrected_dataset.NegativeValueCheck();

            if (negative_elements[sample->first].size() == 0)
            {
                // No negative values - proceed with CMB solution
                corrected_dataset.InitializeParametersAndObservations(sample->first);

                // Update progress with sample name
                if (rtw_)
                {
                    rtw_->SetProgress(double(valid_sample_count) / double(at(target_group_).size()));
                    rtw_->SetLabel(QString::fromStdString(sample->first));
                }

                // Solve CMB model
                corrected_dataset.SolveLevenberg_Marquardt(transform);

                // Record contributions and label with sample name
                contribution_results.append(valid_sample_count, corrected_dataset.GetContributionVector().vec);
                contribution_results.SetLabel(valid_sample_count, sample->first);
                valid_sample_count++;
            }
        }
    }

    // Set progress to 100% at completion
    if (rtw_)
    {
        rtw_->SetProgress(1.0);
    }

    return contribution_results;
}


vector<string> SourceSinkData::AllSourceSampleNames() const
{
    vector<string> all_sample_names;

    // Collect sample names from all source groups
    for (map<string, Elemental_Profile_Set>::const_iterator profile_set = cbegin();
        profile_set != cend();
        profile_set++)
    {
        // Skip target group
        if (profile_set->first != target_group_)
        {
            // Add all sample names from this source group
            for (map<string, Elemental_Profile>::const_iterator profile = profile_set->second.cbegin();
                profile != profile_set->second.cend();
                profile++)
            {
                all_sample_names.push_back(profile->first);
            }
        }
    }

    return all_sample_names;
}

vector<string> SourceSinkData::RandomlypickSamples(const double& percentage) const
{
    // Get all source sample names
    vector<string> all_samples = AllSourceSampleNames();
    vector<string> selected_samples;

    // Perform Bernoulli sampling: each sample included independently with probability = percentage
    for (size_t i = 0; i < all_samples.size(); i++)
    {
        // Generate random number in [0, 1)
        double random_value = GADistribution::GetRndUniF(0, 1);

        // Include sample if random value < percentage
        if (random_value < percentage)
        {
            selected_samples.push_back(all_samples[i]);
        }
    }

    return selected_samples;
}

Results SourceSinkData::MCMC(
    const string& target_sample,
    map<string, string> arguments,
    CMCMC<SourceSinkData>* mcmc,
    ProgressWindow* progress_window,
    const string& working_folder)
{
    // Initialize results container
    Results results;
    results.SetName("MCMC results for '" + target_sample + "'");

    // Parse OM/size correction setting
    bool apply_om_size_correction = (arguments["Apply size and organic matter correction"] == "true");

    // Apply corrections and validate
    SourceSinkData corrected_data = CreateCorrectedDataset(
        target_sample, apply_om_size_correction, GetElementInformation());
    vector<string> negative_elements = corrected_data.NegativeValueCheck();

    if (negative_elements.size() > 0)
    {
        // Negative values detected - return error
        results.SetError("Negative elemental content in ");
        for (size_t i = 0; i < negative_elements.size(); i++)
        {
            if (i == 0)
                results.AppendError(negative_elements[i]);
            else
                results.AppendError("," + negative_elements[i]);
        }
        return results;
    }

    // Configure MCMC sampler
    mcmc->Model = &corrected_data;

    // Setup progress window charts
    progress_window->SetTitle("Acceptance Rate", 0);
    progress_window->SetTitle("Purturbation Factor", 1);
    progress_window->SetTitle("Log posterior value", 2);
    progress_window->SetYAxisTitle("Acceptance Rate", 0);
    progress_window->SetYAxisTitle("Purturbation Factor", 1);
    progress_window->SetYAxisTitle("Log posterior value", 2);
    progress_window->show();

    // ========== Run MCMC Sampling ==========
    corrected_data.InitializeParametersAndObservations(target_sample);

    // Set MCMC parameters
    mcmc->SetProperty("number_of_samples", arguments["Number of samples"]);
    mcmc->SetProperty("number_of_chains", arguments["Number of chains"]);
    mcmc->SetProperty("number_of_burnout_samples", arguments["Samples to be discarded (burnout)"]);
    mcmc->SetProperty("dissolve_chains", arguments["Dissolve Chains"]);

    // Initialize MCMC sampler
    CMBTimeSeriesSet* mcmc_samples = new CMBTimeSeriesSet();
    mcmc->initialize(mcmc_samples, true);

    // Determine output file path
    string output_path;
    if (!QString::fromStdString(arguments["Samples File Name"]).contains("/"))
    {
        output_path = working_folder + "/";
    }

    // Run MCMC chains
    int num_chains = QString::fromStdString(arguments["Number of chains"]).toInt();
    int num_samples = QString::fromStdString(arguments["Number of samples"]).toInt();
    mcmc->step(num_chains, num_samples, output_path + arguments["Samples File Name"],
        mcmc_samples, progress_window);

    // Append last contribution (computed from sum constraint)
    vector<string> source_names = corrected_data.SourceGroupNames();
    size_t last_source_idx = source_names.size() - 1;
    mcmc_samples->AppendLastContribution(last_source_idx,
        source_names[last_source_idx] + "_contribution");

    // Store MCMC samples result
    ResultItem mcmc_samples_result;
    mcmc_samples_result.SetShowAsString(false);
    mcmc_samples_result.SetType(result_type::mcmc_samples);
    mcmc_samples_result.SetName("MCMC samples");
    mcmc_samples_result.SetResult(mcmc_samples);
    results.Append(mcmc_samples_result);

    // Parse burnin parameter
    int burnin_samples = QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt();

    // ========== Result 1: Posterior Distributions for Contributions ==========
    ResultItem posterior_distributions_result;
    CMBTimeSeriesSet* posterior_distributions = new CMBTimeSeriesSet();
    *posterior_distributions = mcmc_samples->distribution(100, burnin_samples);

    posterior_distributions_result.SetName("Posterior Distributions");
    posterior_distributions_result.SetShowAsString(false);
    posterior_distributions_result.SetShowTable(true);
    posterior_distributions_result.SetType(result_type::distribution);
    posterior_distributions_result.SetResult(posterior_distributions);
    results.Append(posterior_distributions_result);

    // ========== Result 2: Contribution Credible Intervals ==========
    RangeSet* contribution_credible_intervals = new RangeSet();

    for (size_t i = 0; i < corrected_data.GetSourceOrder().size(); i++)
    {
        Range interval;

        // Compute statistics (excluding burnin)
        double percentile_2_5 = mcmc_samples->at(i).percentile(0.025, burnin_samples);
        double percentile_97_5 = mcmc_samples->at(i).percentile(0.975, burnin_samples);
        double mean_contribution = mcmc_samples->at(i).mean(burnin_samples);
        double median_contribution = mcmc_samples->at(i).percentile(0.5, burnin_samples);

        interval.Set(_range::low, percentile_2_5);
        interval.Set(_range::high, percentile_97_5);
        interval.SetMean(mean_contribution);
        interval.SetMedian(median_contribution);

        (*contribution_credible_intervals)[mcmc_samples->getSeriesName(i)] = interval;
    }

    ResultItem contribution_intervals_result;
    contribution_intervals_result.SetName("Source Contribution Credible Intervals");
    contribution_intervals_result.SetShowAsString(true);
    contribution_intervals_result.SetShowTable(true);
    contribution_intervals_result.SetType(result_type::rangeset);
    contribution_intervals_result.SetResult(contribution_credible_intervals);
    contribution_intervals_result.SetYAxisMode(yaxis_mode::log);
    contribution_intervals_result.SetYLimit(_range::high, 1.0);
    results.Append(contribution_intervals_result);

    // ========== Result 3: Predicted Element Distributions ==========
    CMBTimeSeriesSet predicted_samples = mcmc->predicted;
    vector<string> element_names = corrected_data.ElementOrder();
    vector<string> isotope_names = corrected_data.IsotopeOrder();
    vector<string> all_constituent_names = element_names;
    all_constituent_names.insert(all_constituent_names.end(), isotope_names.begin(), isotope_names.end());

    // Set constituent names
    for (size_t i = 0; i < predicted_samples.size(); i++)
    {
        predicted_samples.setname(i, all_constituent_names[i]);
    }

    // Extract element predictions only
    CMBTimeSeriesSet predicted_elements;
    for (size_t i = 0; i < predicted_samples.size(); i++)
    {
        if (corrected_data.GetElementInformation(predicted_samples.getSeriesName(i))->Role ==
            element_information::role::element)
        {
            predicted_elements.append(predicted_samples[i], predicted_samples.getSeriesName(i));
        }
    }

    // Compute posterior distributions for elements
    CMBTimeSeriesSet* predicted_element_distributions = new CMBTimeSeriesSet();
    *predicted_element_distributions = predicted_elements.distribution(100, burnin_samples);

    // Add observed values
    for (size_t i = 0; i < predicted_elements.size(); i++)
    {
        predicted_element_distributions->SetObservedValue(i, corrected_data.observation(i)->Value());
    }

    ResultItem predicted_elements_result;
    predicted_elements_result.SetName("Posterior Predicted Constituents");
    predicted_elements_result.SetShowAsString(false);
    predicted_elements_result.SetShowTable(true);
    predicted_elements_result.SetType(result_type::distribution_with_observed);
    predicted_elements_result.SetResult(predicted_element_distributions);
    results.Append(predicted_elements_result);

    // ========== Result 4: Element Credible Intervals ==========
    RangeSet* predicted_element_intervals = new RangeSet();

    // Compute percentiles for all predictions
    vector<double> percentile_2_5_all = predicted_samples.percentile(0.025, burnin_samples);
    vector<double> percentile_97_5_all = predicted_samples.percentile(0.975, burnin_samples);
    vector<double> mean_all = predicted_samples.mean(burnin_samples);
    vector<double> median_all = predicted_samples.percentile(0.5, burnin_samples);

    for (size_t i = 0; i < predicted_element_distributions->size(); i++)
    {
        Range interval;
        interval.Set(_range::low, percentile_2_5_all[i]);
        interval.Set(_range::high, percentile_97_5_all[i]);
        interval.SetMean(mean_all[i]);
        interval.SetMedian(median_all[i]);

        (*predicted_element_intervals)[predicted_element_distributions->getSeriesName(i)] = interval;
        (*predicted_element_intervals)[predicted_element_distributions->getSeriesName(i)].SetValue(
            corrected_data.observation(i)->Value());
    }

    ResultItem predicted_element_intervals_result;
    predicted_element_intervals_result.SetName("Predicted Samples Credible Intervals");
    predicted_element_intervals_result.SetShowAsString(true);
    predicted_element_intervals_result.SetShowTable(true);
    predicted_element_intervals_result.SetType(result_type::rangeset_with_observed);
    predicted_element_intervals_result.SetResult(predicted_element_intervals);
    predicted_element_intervals_result.SetYAxisMode(yaxis_mode::log);
    results.Append(predicted_element_intervals_result);

    // ========== Result 5: Predicted Isotope Distributions ==========
    CMBTimeSeriesSet predicted_isotopes;

    for (size_t i = 0; i < predicted_samples.size(); i++)
    {
        if (corrected_data.GetElementInformation(predicted_samples.getSeriesName(i))->Role ==
            element_information::role::isotope)
        {
            predicted_isotopes.append(predicted_samples[i], predicted_samples.getSeriesName(i));
        }
    }

    CMBTimeSeriesSet* predicted_isotope_distributions = new CMBTimeSeriesSet();
    *predicted_isotope_distributions = predicted_isotopes.distribution(100, burnin_samples);

    // Add observed values
    for (size_t i = 0; i < predicted_isotopes.size(); i++)
    {
        predicted_isotope_distributions->SetObservedValue(
            i, corrected_data.observation(i + element_names.size())->Value());
    }

    ResultItem predicted_isotopes_result;
    predicted_isotopes_result.SetName("Posterior Predicted Isotopes");
    predicted_isotopes_result.SetShowAsString(false);
    predicted_isotopes_result.SetShowTable(true);
    predicted_isotopes_result.SetType(result_type::distribution_with_observed);
    predicted_isotopes_result.SetResult(predicted_isotope_distributions);
    results.Append(predicted_isotopes_result);

    // ========== Result 6: Isotope Credible Intervals ==========
    RangeSet* predicted_isotope_intervals = new RangeSet();
    size_t element_offset = corrected_data.ElementOrder().size();

    for (size_t i = 0; i < predicted_isotope_distributions->size(); i++)
    {
        Range interval;
        interval.Set(_range::low, percentile_2_5_all[i + element_offset]);
        interval.Set(_range::high, percentile_97_5_all[i + element_offset]);
        interval.SetMean(mean_all[i + element_offset]);
        interval.SetMedian(median_all[i + element_offset]);

        (*predicted_isotope_intervals)[predicted_isotope_distributions->getSeriesName(i)] = interval;
        (*predicted_isotope_intervals)[predicted_isotope_distributions->getSeriesName(i)].SetValue(
            corrected_data.observation(i + element_offset)->Value());
    }

    ResultItem predicted_isotope_intervals_result;
    predicted_isotope_intervals_result.SetName("Predicted Samples Credible Intervals for Isotopes");
    predicted_isotope_intervals_result.SetShowAsString(true);
    predicted_isotope_intervals_result.SetShowTable(true);
    predicted_isotope_intervals_result.SetType(result_type::rangeset_with_observed);
    predicted_isotope_intervals_result.SetResult(predicted_isotope_intervals);
    predicted_isotope_intervals_result.SetYAxisMode(yaxis_mode::normal);
    results.Append(predicted_isotope_intervals_result);

    // Set progress to 100%
    progress_window->SetProgress(1.0);

    return results;
}


CMBMatrix SourceSinkData::MCMC_Batch(
    map<string, string> arguments,
    CMCMC<SourceSinkData>* mcmc,
    ProgressWindow* progress_window,
    const string& working_folder)
{
    // Initialize with first target sample
    InitializeParametersAndObservations(at(target_group_).begin()->first);

    // Initialize contribution matrix: 4 columns per source (low, high, median, mean)
    const size_t num_target_samples = at(target_group_).size();
    const size_t num_columns = numberofsourcesamplesets_ * 4;
    CMBMatrix contribution_statistics(num_columns, num_target_samples);

    // Set column labels
    for (size_t source_idx = 0; source_idx < numberofsourcesamplesets_; source_idx++)
    {
        const string& source_name = samplesetsorder_[source_idx];
        contribution_statistics.SetColumnLabel(source_idx * 4, source_name + "-low");
        contribution_statistics.SetColumnLabel(source_idx * 4 + 1, source_name + "-high");
        contribution_statistics.SetColumnLabel(source_idx * 4 + 2, source_name + "-median");
        contribution_statistics.SetColumnLabel(source_idx * 4 + 3, source_name + "-mean");
    }

    // Process each target sample
    size_t sample_counter = 0;
    for (map<string, Elemental_Profile>::iterator sample = at(target_group_).begin();
        sample != at(target_group_).end();
        sample++)
    {
        const string& sample_name = sample->first;

        // Create output directory for this sample
        QDir sample_dir(QString::fromStdString(working_folder) + "/" + QString::fromStdString(sample_name));
        if (!sample_dir.exists())
        {
            sample_dir.mkpath(".");
        }

        // Set matrix row label
        contribution_statistics.SetRowLabel(sample_counter, sample_name);

        // Update progress label
        progress_window->SetLabel(QString::fromStdString(sample_name));

        // Run MCMC analysis for this sample
        Results mcmc_results = MCMC(sample_name, arguments, mcmc, progress_window, working_folder);

        // Save all result items to text files
        for (map<string, ResultItem>::iterator result_item = mcmc_results.begin();
            result_item != mcmc_results.end();
            result_item++)
        {
            QString file_path = sample_dir.absolutePath() + "/" +
                QString::fromStdString(result_item->first) + ".txt";
            QFile output_file(file_path);
            output_file.open(QIODevice::WriteOnly | QIODevice::Text);
            result_item->second.Result()->writetofile(&output_file);
            output_file.close();
        }

        // Extract contribution statistics from credible intervals
        RangeSet* credible_intervals = static_cast<RangeSet*>(
            mcmc_results.at("3:Source Contribution Credible Intervals").Result());

        for (size_t source_idx = 0; source_idx < numberofsourcesamplesets_; source_idx++)
        {
            const string contribution_name = samplesetsorder_[source_idx] + "_contribution";
            const Range& contribution_interval = credible_intervals->at(contribution_name);

            // Store statistics: low (2.5%), high (97.5%), median, mean
            contribution_statistics[sample_counter][4 * source_idx] = contribution_interval.Get(_range::low);
            contribution_statistics[sample_counter][4 * source_idx + 1] = contribution_interval.Get(_range::high);
            contribution_statistics[sample_counter][4 * source_idx + 2] = contribution_interval.Median();
            contribution_statistics[sample_counter][4 * source_idx + 3] = contribution_interval.Mean();
        }

        sample_counter++;

        // Update batch progress
        progress_window->SetProgress2(double(sample_counter) / double(num_target_samples));

        // Clear MCMC progress charts for next sample
        progress_window->ClearGraph(0);
        progress_window->ClearGraph(1);
        progress_window->ClearGraph(2);
    }

    // Set batch progress to 100%
    progress_window->SetProgress2(1.0);

    return contribution_statistics;
}
bool SourceSinkData::ToolsUsed(const string& tool_name)
{
    // Search for tool in the tools_used list
    std::list<string>::iterator iter = std::find(tools_used_.begin(), tools_used_.end(), tool_name);

    // Return true if found, false if not found
    return (iter != tools_used_.end());
}

string SourceSinkData::FirstOMConstituent()
{
    // Search for first organic carbon constituent
    for (map<string, element_information>::iterator element = element_information_.begin();
        element != element_information_.end();
        element++)
    {
        if (element->second.Role == element_information::role::organic_carbon)
        {
            return element->first;
        }
    }

    // No organic carbon constituent found
    return "";
}

string SourceSinkData::FirstSizeConstituent()
{
    // Search for first particle size constituent
    for (map<string, element_information>::iterator element = element_information_.begin();
        element != element_information_.end();
        element++)
    {
        if (element->second.Role == element_information::role::particle_size)
        {
            return element->first;
        }
    }

    // No particle size constituent found
    return "";
}

CMatrix SourceSinkData::WithinGroupCovarianceMatrix()
{
    vector<string> element_names = GetElementNames();
    CMatrix within_group_covariance(element_names.size());

    // Accumulate weighted covariance matrices from each source group
    int total_degrees_of_freedom = 0;
    for (map<string, Elemental_Profile_Set>::iterator source_group = begin();
        source_group != end();
        source_group++)
    {
        if (source_group->first != target_group_)
        {
            // Weight by degrees of freedom (n - 1) for this group
            int group_df = source_group->second.size() - 1;
            within_group_covariance += group_df * source_group->second.CalculateCovarianceMatrix();
            total_degrees_of_freedom += group_df;
        }
    }

    // Return pooled within-group covariance
    return within_group_covariance / double(total_degrees_of_freedom);
}

CMatrix SourceSinkData::BetweenGroupCovarianceMatrix()
{
    vector<string> element_names = GetElementNames();
    CMatrix between_group_covariance(element_names.size());

    // Get overall mean across all sources
    CMBVector overall_mean = MeanElementalContent();

    // Accumulate weighted outer products of group mean deviations
    double total_samples = 0.0;
    for (map<string, Elemental_Profile_Set>::iterator source_group = begin();
        source_group != end();
        source_group++)
    {
        if (source_group->first != target_group_)
        {
            // Compute deviation of group mean from overall mean
            CMBVector group_mean = MeanElementalContent(source_group->first);
            CMBVector deviation = overall_mean - group_mean;

            // Add weighted outer product: n_i × (μ_i - μ)(μ_i - μ)ᵀ
            size_t group_size = source_group->second.size();
            for (size_t i = 0; i < element_names.size(); i++)
            {
                for (size_t j = 0; j < element_names.size(); j++)
                {
                    between_group_covariance[i][j] += deviation[i] * deviation[j] * group_size;
                }
            }

            total_samples += group_size;
        }
    }

    // Return normalized between-group covariance
    return between_group_covariance / total_samples;
}

CMatrix SourceSinkData::TotalScatterMatrix()
{
    vector<string> element_names = GetElementNames();
    CMatrix total_scatter(element_names.size());

    // Get overall mean across all sources
    CMBVector overall_mean = MeanElementalContent();

    // Accumulate sum of squared deviations from overall mean
    double total_samples = 0.0;
    for (map<string, Elemental_Profile_Set>::iterator source_group = begin();
        source_group != end();
        source_group++)
    {
        if (source_group->first != target_group_)
        {
            // Process each sample in this group
            for (map<string, Elemental_Profile>::iterator sample = source_group->second.begin();
                sample != source_group->second.end();
                sample++)
            {
                // Add outer product of deviation: (x - μ)(x - μ)ᵀ
                for (size_t i = 0; i < element_names.size(); i++)
                {
                    for (size_t j = 0; j < element_names.size(); j++)
                    {
                        double deviation_i = overall_mean[i] - sample->second.at(element_names[i]);
                        double deviation_j = overall_mean[j] - sample->second.at(element_names[j]);
                        total_scatter[i][j] += deviation_i * deviation_j;
                    }
                }
            }

            total_samples += source_group->second.size();
        }
    }

    // Return normalized total scatter matrix
    return total_scatter / total_samples;
}

double SourceSinkData::WilksLambda()
{
    // Get covariance matrices
    CMatrix_arma within_group_cov = WithinGroupCovarianceMatrix();
    CMatrix_arma between_group_cov = BetweenGroupCovarianceMatrix();
    CMatrix_arma total_cov = within_group_cov + between_group_cov;

    // Compute Wilks' Lambda = |S_W| / |S_T|
    double numerator = within_group_cov.det();
    double denominator = total_cov.det();

    // Use absolute values for numerical stability
    return fabs(numerator) / fabs(denominator);
}

double SourceSinkData::DFA_P_Value()
{
    int num_elements = GetElementNames().size();

    // Get Wilks' Lambda (capped at 1.0)
    double wilks_lambda = min(WilksLambda(), 1.0);

    // Transform to chi-squared statistic
    double sample_size = TotalNumberofSourceSamples();
    double correction_factor = (num_elements + (numberofsourcesamplesets_ - 1.0)) / 2.0;
    double chi_squared = -(sample_size - 1.0 - correction_factor) * log(wilks_lambda);

    // Compute degrees of freedom
    double degrees_of_freedom;
    if (target_group_ != "")
    {
        // Target group present: exclude from group count
        degrees_of_freedom = num_elements * (numberofsourcesamplesets_ - 2.0);
    }
    else
    {
        // No target group
        degrees_of_freedom = num_elements * (numberofsourcesamplesets_ - 1.0);
    }

    // Compute p-value from chi-squared distribution
    double p_value = gsl_cdf_chisq_Q(chi_squared, degrees_of_freedom);

    return p_value;
}

CMBVectorSet SourceSinkData::DFA_Projected()
{
    // Get discriminant eigenvector
    CMBVector discriminant_function = DFA_eigvector();

    CMBVectorSet projected_scores;

    // Project each group onto discriminant axis
    for (map<string, Elemental_Profile_Set>::iterator source_group = begin();
        source_group != end();
        source_group++)
    {
        // Compute discriminant scores for this group
        CMBVector group_scores = source_group->second.CalculateDotProduct(discriminant_function);
        projected_scores.Append(source_group->first, group_scores);
    }

    return projected_scores;
}

CMBVectorSet SourceSinkData::DFA_Projected(const string& source1, const string& source2)
{
    // Get discriminant eigenvector
    CMBVector discriminant_function = DFA_eigvector();

    CMBVectorSet projected_scores;

    // Project each group onto discriminant axis
    // Note: Parameters source1 and source2 not currently used
    for (map<string, Elemental_Profile_Set>::iterator source_group = begin();
        source_group != end();
        source_group++)
    {
        // Compute discriminant scores for this group
        CMBVector group_scores = source_group->second.CalculateDotProduct(discriminant_function);
        projected_scores.Append(source_group->first, group_scores);
    }

    return projected_scores;
}

CMBVectorSet SourceSinkData::DFA_Projected(const string& source1, SourceSinkData* original)
{
    // Get discriminant eigenvector from current (reduced) dataset
    CMBVector discriminant_function = DFA_eigvector();

    CMBVectorSet projected_scores;

    // Project samples from original dataset onto discriminant axis
    for (map<string, Elemental_Profile_Set>::iterator source_group = original->begin();
        source_group != original->end();
        source_group++)
    {
        // Skip target group
        if (source_group->first != original->target_group_)
        {
            // Compute discriminant scores for this group
            CMBVector group_scores = source_group->second.CalculateDotProduct(discriminant_function);
            projected_scores.Append(source_group->first, group_scores);
        }
    }

    return projected_scores;
}

CMBVector SourceSinkData::DFA_eigvector()
{
    // Get covariance matrices
    CMatrix_arma between_group_cov = BetweenGroupCovarianceMatrix();
    CMatrix_arma within_group_cov = WithinGroupCovarianceMatrix();

    // Invert within-group covariance
    CMatrix_arma inv_within_cov = inv(within_group_cov);

    // Check if inversion failed (singular matrix)
    if (inv_within_cov.getnumrows() == 0)
    {
        return CMBVector();  // Return empty vector
    }

    // Compute product matrix for eigenvalue problem
    CMatrix_arma product_matrix = inv_within_cov * between_group_cov;

    // Solve eigenvalue problem
    arma::cx_vec eigenvalues;
    arma::cx_mat eigenvectors;
    eig_gen(eigenvalues, eigenvectors, product_matrix);

    // Extract real parts
    CVector_arma real_eigenvalues = GetReal(eigenvalues);
    CMatrix_arma real_eigenvectors = GetReal(eigenvectors);

    // Find eigenvector corresponding to largest eigenvalue (by absolute value)
    size_t max_eigenvalue_index = real_eigenvalues.abs_max_elems();
    CMBVector discriminant_function = CVector_arma(real_eigenvectors.getcol(max_eigenvalue_index));

    // Label with element names
    vector<string> element_names = GetElementNames();
    discriminant_function.SetLabels(element_names);

    return discriminant_function;
}

CMBVector SourceSinkData::DFA_weight_vector(const string& source1, const string& source2)
{
    // Get covariance matrices for both groups
    CMatrix_arma cov_matrix1 = at(source1).CalculateCovarianceMatrix();
    CMatrix_arma cov_matrix2 = at(source2).CalculateCovarianceMatrix();

    // Get mean vectors for both groups
    CVector mean_vector1 = at(source1).CalculateElementMeans();
    CVector mean_vector2 = at(source2).CalculateElementMeans();
    CVector_arma mean1 = mean_vector1;
    CVector_arma mean2 = mean_vector2;

    // Compute Fisher's linear discriminant: w = (μ₂ - μ₁) / (Σ₁ + Σ₂)
    CMatrix_arma pooled_cov = cov_matrix1 + cov_matrix2;
    CVector weight_vector_arma = (mean2 - mean1) / pooled_cov;

    // Convert to CMBVector and normalize
    CMBVector weight_vector = weight_vector_arma;
    weight_vector = weight_vector / weight_vector.norm2();

    // Label with element names
    weight_vector.SetLabels(GetElementNames());

    return weight_vector;
}

CMBVector SourceSinkData::MeanElementalContent(const string& group_name)
{
    // Check if group exists
    if (count(group_name) == 0)
    {
        return CMBVector();  // Return empty vector
    }

    // Compute mean concentrations for this group
    vector<string> element_names = GetElementNames();
    CMBVector group_mean = at(group_name).CalculateElementMeans();
    group_mean.SetLabels(element_names);

    return group_mean;
}

CMBVector SourceSinkData::MeanElementalContent()
{
    vector<string> element_names = GetElementNames();
    CMBVector weighted_mean(element_names.size());

    // Accumulate weighted means from each source group
    double total_samples = 0.0;
    for (map<string, Elemental_Profile_Set>::iterator source_group = begin();
        source_group != end();
        source_group++)
    {
        // Skip target group
        if (source_group->first != target_group_)
        {
            // Weight group mean by sample size
            size_t group_size = source_group->second.size();
            CMBVector group_mean = MeanElementalContent(source_group->first);

            weighted_mean += double(group_size) * group_mean;
            total_samples += double(group_size);
        }
    }

    // Set labels and normalize by total sample count
    weighted_mean.SetLabels(element_order_);

    return weighted_mean / total_samples;
}

vector<CMBVector> SourceSinkData::StepwiseDiscriminantFunctionAnalysis(
    const string& source1,
    const string& source2)
{
    // Initialize result vectors: [0]=p-values, [1]=Wilks' Lambda, [2]=F-test p-values
    vector<CMBVector> stepwise_results(3);

    vector<string> element_names = GetElementNames();
    vector<string> selected_elements;

    // Forward selection: add one element at a time
    for (size_t step = 0; step < element_names.size(); step++)
    {
        // Update progress
        if (rtw_)
        {
            rtw_->SetProgress(double(step + 1) / double(element_names.size()));
        }

        // Find best element to add next
        double best_p_value = 100.0;
        string best_element;
        double best_wilks_lambda;
        double best_f_test_p_value;

        // Test each unselected element
        for (size_t j = 0; j < element_names.size(); j++)
        {
            // Skip if already selected
            if (lookup(selected_elements, element_names[j]) == -1)
            {
                // Create candidate selection with this element added
                vector<string> candidate_elements = selected_elements;
                candidate_elements.push_back(element_names[j]);

                // Extract dataset with only candidate elements
                SourceSinkData candidate_dataset = ExtractSpecificElements(candidate_elements);

                // Perform DFA on candidate dataset
                DFA_result candidate_dfa = candidate_dataset.DiscriminantFunctionAnalysis(source1, source2);

                // Check if DFA succeeded
                if (candidate_dfa.eigen_vectors.size() == 0)
                {
                    return stepwise_results;  // Return empty results on failure
                }

                // Update best if this element improves discrimination
                if (candidate_dfa.p_values[0] < best_p_value)
                {
                    best_element = element_names[j];
                    best_p_value = candidate_dfa.p_values[0];
                    best_wilks_lambda = candidate_dfa.wilkslambda[0];
                    best_f_test_p_value = candidate_dfa.F_test_P_value[0];
                }
            }
        }

        // Record statistics for best element at this step
        stepwise_results[0].append(best_element, best_p_value);
        stepwise_results[1].append(best_element, best_wilks_lambda);
        stepwise_results[2].append(best_element, best_f_test_p_value);

        // Add best element to selection
        selected_elements.push_back(best_element);
    }

    return stepwise_results;
}

vector<CMBVector> SourceSinkData::StepwiseDiscriminantFunctionAnalysis()
{
    // Initialize result vectors: [0]=p-values, [1]=Wilks' Lambda, [2]=F-test p-values
    vector<CMBVector> stepwise_results(3);

    vector<string> element_names = GetElementNames();
    vector<string> selected_elements;

    // Forward selection: add one element at a time
    for (size_t step = 0; step < element_names.size(); step++)
    {
        // Update progress
        if (rtw_)
        {
            rtw_->SetProgress(double(step + 1) / double(element_names.size()));
        }

        // Find best element to add next
        double best_p_value = 100.0;
        string best_element;
        double best_wilks_lambda;
        double best_f_test_p_value;

        // Test each unselected element
        for (size_t j = 0; j < element_names.size(); j++)
        {
            // Skip if already selected
            if (lookup(selected_elements, element_names[j]) == -1)
            {
                // Create candidate selection with this element added
                vector<string> candidate_elements = selected_elements;
                candidate_elements.push_back(element_names[j]);

                // Extract dataset with only candidate elements
                SourceSinkData candidate_dataset = ExtractSpecificElements(candidate_elements);

                // Compute multi-group DFA statistics
                double p_value = candidate_dataset.DFA_P_Value();

                // Update best if this element improves discrimination
                if (p_value < best_p_value)
                {
                    best_element = element_names[j];
                    best_p_value = p_value;
                    best_wilks_lambda = candidate_dataset.WilksLambda();

                    // Compute F-test p-value from projections
                    CMBVectorSet projected = candidate_dataset.DFA_Projected();
                    best_f_test_p_value = projected.FTest_p_value();
                }
            }
        }

        // Record statistics for best element at this step
        stepwise_results[0].append(best_element, best_p_value);
        stepwise_results[1].append(best_element, best_wilks_lambda);
        stepwise_results[2].append(best_element, best_f_test_p_value);

        // Add best element to selection
        selected_elements.push_back(best_element);
    }

    return stepwise_results;
}

vector<CMBVector> SourceSinkData::StepwiseDiscriminantFunctionAnalysis(const string& source1)
{
    // Initialize result vectors: [0]=p-values, [1]=Wilks' Lambda, [2]=F-test p-values
    vector<CMBVector> stepwise_results(3);

    vector<string> element_names = GetElementNames();
    vector<string> selected_elements;

    // Forward selection: add one element at a time
    for (size_t step = 0; step < element_names.size(); step++)
    {
        // Update progress
        if (rtw_)
        {
            rtw_->SetProgress(double(step + 1) / double(element_names.size()));
        }

        // Find best element to add next
        double best_p_value = 100.0;
        string best_element;
        double best_wilks_lambda;
        double best_f_test_p_value;

        // Test each unselected element
        for (size_t j = 0; j < element_names.size(); j++)
        {
            // Skip if already selected
            if (lookup(selected_elements, element_names[j]) == -1)
            {
                // Create candidate selection with this element added
                vector<string> candidate_elements = selected_elements;
                candidate_elements.push_back(element_names[j]);

                // Extract dataset with only candidate elements
                SourceSinkData candidate_dataset = ExtractSpecificElements(candidate_elements);

                // Perform one-vs-rest DFA on candidate dataset
                DFA_result candidate_dfa = candidate_dataset.DiscriminantFunctionAnalysis(source1);

                // Update best if this element improves discrimination
                if (candidate_dfa.p_values[0] < best_p_value)
                {
                    best_element = element_names[j];
                    best_p_value = candidate_dfa.p_values[0];
                    best_wilks_lambda = candidate_dfa.wilkslambda[0];
                    best_f_test_p_value = candidate_dfa.F_test_P_value[0];
                }
            }
        }

        // Record statistics for best element at this step
        stepwise_results[0].append(best_element, best_p_value);
        stepwise_results[1].append(best_element, best_wilks_lambda);
        stepwise_results[2].append(best_element, best_f_test_p_value);

        // Add best element to selection
        selected_elements.push_back(best_element);
    }

    return stepwise_results;
}

// ========================================
// METHOD DEFINITIONS TO MOVE TO sourcesinkdata.cpp
// ========================================
// These were previously inline in the header and should now be implemented in the .cpp file

// ========== Accessors ==========

vector<Parameter>& SourceSinkData::Parameters()
{
    return parameters_;
}

size_t SourceSinkData::ParametersCount()
{
    return parameters_.size();
}

size_t SourceSinkData::ObservationsCount()
{
    return observations_.size();
}

Parameter* SourceSinkData::parameter(size_t i)
{
    if (i >= 0 && i < parameters_.size())
        return &parameters_[i];
    else
        return nullptr;
}

const Parameter* SourceSinkData::parameter(size_t i) const
{
    if (i >= 0 && i < parameters_.size())
        return &parameters_[i];
    else
        return nullptr;
}

Observation* SourceSinkData::observation(size_t i)
{
    if (i >= 0 && i < observations_.size())
        return &observations_[i];
    else
        return nullptr;
}

bool SourceSinkData::SetTargetGroup(const string& targroup)
{
    target_group_ = targroup;
    return true;
}

string SourceSinkData::GetTargetGroup()
{
    return target_group_;
}

void SourceSinkData::SetParameterEstimationMode(estimation_mode est_mode)
{
    parameter_estimation_mode_ = est_mode;
}

estimation_mode SourceSinkData::ParameterEstimationMode()
{
    return parameter_estimation_mode_;
}

void SourceSinkData::SetProgressWindow(ProgressWindow* _rtw)
{
    rtw_ = _rtw;
}

map<string, element_information>* SourceSinkData::GetElementInformation()
{
    return &element_information_;
}

element_information* SourceSinkData::GetElementInformation(const string& element_name)
{
    if (element_information_.count(element_name))
        return &element_information_.at(element_name);
    else
        return nullptr;
}

ConcentrationSet* SourceSinkData::GetElementDistribution(const string& element_name)
{
    if (element_distributions_.count(element_name))
        return &element_distributions_.at(element_name);
    else
        return nullptr;
}

ConcentrationSet* SourceSinkData::GetElementDistribution(const string& element_name, const string& sample_group)
{
    if (!GetSampleSet(sample_group))
    {
        cout << "Sample Group '" + sample_group + "' does not exist!" << std::endl;
        return nullptr;
    }
    if (!GetSampleSet(sample_group)->GetElementDistribution(element_name))
    {
        cout << "Element '" + element_name + "' does not exist!" << std::endl;
        return nullptr;
    }
    return GetSampleSet(sample_group)->GetElementDistribution(element_name);
}

// ========== Ordering Accessors ==========

vector<string> SourceSinkData::GetSourceOrder() const
{
    return samplesetsorder_;
}

vector<string> SourceSinkData::SamplesetsOrder()
{
    return samplesetsorder_;
}

vector<string> SourceSinkData::ConstituentOrder()
{
    return constituent_order_;
}

vector<string> SourceSinkData::ElementOrder()
{
    return element_order_;
}

vector<string> SourceSinkData::IsotopeOrder()
{
    return isotope_order_;
}

vector<string> SourceSinkData::SizeOMOrder()
{
    return size_om_order_;
}

// ========== OM/Size Constituent Management ==========

void SourceSinkData::SetOMandSizeConstituents(const string& _omconstituent, const string& _sizeconsituent)
{
    omconstituent_ = _omconstituent;
    sizeconsituent_ = _sizeconsituent;
}

void SourceSinkData::SetOMandSizeConstituents(const vector<string>& _omsizeconstituents)
{
    if (_omsizeconstituents.size() == 0)
        return;
    else if (_omsizeconstituents.size() == 1)
        omconstituent_ = _omsizeconstituents[0];
    else if (_omsizeconstituents.size() == 2)
    {
        omconstituent_ = _omsizeconstituents[0];
        sizeconsituent_ = _omsizeconstituents[1];
    }
}

vector<string> SourceSinkData::OMandSizeConstituents()
{
    vector<string> out;
    out.push_back(omconstituent_);
    out.push_back(sizeconsituent_);
    return out;
}

// ========== Options Management ==========

QMap<QString, double>* SourceSinkData::GetOptions()
{
    return &options_;
}

// ========== Output Path Management ==========

string SourceSinkData::GetOutputPath() const
{
    return outputpath_;
}

bool SourceSinkData::SetOutputPath(const string& output_path)
{
    outputpath_ = output_path;
    return true;
}

// ========== Selected Target Sample Management ==========

bool SourceSinkData::SetSelectedTargetSample(const string& sample_name)
{
    // Validate that sample exists in target group
    if (count(target_group_) == 0)
        return false;

    if (at(target_group_).count(sample_name) == 0)
        return false;

    selected_target_sample_ = sample_name;
    return true;
}

string SourceSinkData::SelectedTargetSample() const
{
    return selected_target_sample_;
}