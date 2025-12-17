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

bool SourceSinkData::Execute(const string &command, const map<string,string> &arguments)
{
    return true;
}

string SourceSinkData::GetOutputPath() const
{
    return outputpath_;
}

bool SourceSinkData::SetOutputPath(const string &oppath)
{
    outputpath_ = oppath;
    return true;
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
    double LogLikelihood = 0;
    CVector C;
    if (est_mode == estimation_mode::elemental_profile_and_contribution)
        C = PredictTarget(parameter_mode::based_on_fitted_distribution);
    else
        C = PredictTarget(parameter_mode::direct);
    CVector C_obs = ObservedDataforSelectedSample(selected_target_sample_);

    if (C.min()>0)
        LogLikelihood -= C.num*log(error_stdev_) + pow((C.Log()-C_obs.Log()).norm2(),2)/(2*pow(error_stdev_,2));
    else
        LogLikelihood -= 1e10;


    return LogLikelihood;
}

double SourceSinkData::LogLikelihoodModelvsMeasured_Isotope(estimation_mode est_mode)
{
    double LogLikelihood = 0;
    CVector C;
    if (est_mode == estimation_mode::elemental_profile_and_contribution)
        C = PredictTarget_Isotope_delta(parameter_mode::based_on_fitted_distribution);
    else
        C = PredictTarget_Isotope_delta(parameter_mode::direct);
    CVector C_obs = ObservedDataforSelectedSample_Isotope_delta(selected_target_sample_);


        LogLikelihood -= C.num*log(error_stdev_isotope_) + pow((C-C_obs).norm2(),2)/(2*pow(error_stdev_isotope_,2));

    return LogLikelihood;
}


CVector SourceSinkData::ResidualVector()
{
    CVector C = PredictTarget(parameter_mode::direct);
    CVector C_iso = PredictTarget_Isotope_delta(parameter_mode::direct);
    if (!C.is_finite())
    {
        qDebug()<<"oops!";
    }
    CVector C_obs = ObservedDataforSelectedSample(selected_target_sample_);
    CVector C_obs_iso = ObservedDataforSelectedSample_Isotope_delta(selected_target_sample_);
    CVector Residual = C.Log() - C_obs.Log();
    CVector Residual_isotope = C_iso - C_obs_iso;
    Residual.append(Residual_isotope);
    if (!Residual.is_finite())
    {
        CVector X = GetContributionVector();
        CVector X_softmax = GetContributionVectorSoftmax();
        qDebug()<<"oops!";
    }
    return Residual;
}

CVector_arma SourceSinkData::ResidualVector_arma()
{
    CVector_arma C = PredictTarget().vec;
    CVector_arma C_iso = PredictTarget_Isotope_delta().vec;
    CVector_arma C_obs = ObservedDataforSelectedSample(selected_target_sample_).vec;
    CVector_arma C_obs_iso = ObservedDataforSelectedSample_Isotope_delta(selected_target_sample_).vec;
    CVector_arma Residual = C.Log() - C_obs.Log();
    CVector_arma Residual_iso = C_iso-C_obs;
    Residual.append(Residual_iso);

    return Residual;
}

CMatrix_arma SourceSinkData::ResidualJacobian_arma()
{
    CMatrix_arma Jacobian(GetSourceOrder().size()-1,element_order_.size()+isotope_order_.size());
    CVector_arma base_contribution = GetContributionVector(false).vec;
    CVector_arma base_residual = ResidualVector_arma();
    for (unsigned int i = 0; i < GetSourceOrder().size()-1; i++)
    {
        double epsilon = (0.5 - base_contribution[i]) * 1e-6; 
        SetContribution(i, base_contribution[i] + epsilon);
        CVector_arma purturbed_residual = ResidualVector_arma();
        Jacobian.setcol(i,(purturbed_residual - base_residual) / epsilon);
        SetContribution(i, base_contribution[i]);
    }
    return Jacobian;
}

CMatrix SourceSinkData::ResidualJacobian()
{
    CMatrix Jacobian(GetSourceOrder().size()-1,element_order_.size()+isotope_order_.size());
    CVector base_contribution = GetContributionVector(false).vec;
    CVector base_residual = ResidualVector();
    for (unsigned int i = 0; i < GetSourceOrder().size()-1; i++)
    {
        double epsilon = (0.5 - base_contribution[i]) * 1e-3;
        SetContribution(i, base_contribution[i] + epsilon);
        CVector X = GetContributionVector();
        CVector purturbed_residual = ResidualVector();
        Jacobian.setrow(i,(purturbed_residual - base_residual) / epsilon);
        SetContribution(i, base_contribution[i]);
    }
    return Jacobian;
}

CMatrix SourceSinkData::ResidualJacobian_softmax()
{
    CMatrix Jacobian(GetSourceOrder().size(),element_order_.size()+isotope_order_.size());
    CVector base_contribution = GetContributionVectorSoftmax().vec;
    CVector base_residual = ResidualVector();
    for (unsigned int i = 0; i < GetSourceOrder().size(); i++)
    {
        double epsilon = -sign(base_contribution[i]) * 1e-3;

        CVector X = base_contribution;
        X[i]+=epsilon;
        SetContributionSoftmax(X);
        CVector purturbed_residual = ResidualVector();
        Jacobian.setrow(i,(purturbed_residual - base_residual) / epsilon);
        SetContributionSoftmax(base_contribution);
    }
    return Jacobian;
}


CVector SourceSinkData::OneStepLevenBerg_Marquardt(double lambda)
{

    CVector V = ResidualVector();
    CMatrix M = ResidualJacobian();
    CMatrix JTJ = M*Transpose(M);
    JTJ.ScaleDiagonal(1+lambda);
    CVector J_epsilon = M*V;

    if (det(JTJ)<=1e-6)
    {
        JTJ += lambda*CMatrix::Diag(JTJ.getnumcols());
    }

    CVector dx = J_epsilon/JTJ;
    return dx;
}

CVector SourceSinkData::OneStepLevenBerg_Marquardt_softmax(double lambda)
{

    CVector V = ResidualVector();
    CMatrix M = ResidualJacobian_softmax();
    CMatrix JTJ = M*Transpose(M);
    JTJ.ScaleDiagonal(1+lambda);
    CVector J_epsilon = M*V;

    if (det(JTJ)<=1e-6)
    {
        JTJ += lambda*CMatrix::Diag(JTJ.getnumcols());
    }
    CVector dx = J_epsilon / JTJ;

    return dx;
}


bool SourceSinkData::SolveLevenBerg_Marquardt(transformation trans)
{
    if (trans == transformation::linear)
        InitializeContributionsRandomly();
    else if (trans == transformation::softmax)
        InitializeContributionsRandomlySoftmax();
    double err = 1000;
    double err_p;
    double tol = 1e-10;
    double lambda = 1;
    int counter = 0;
    double err_0 = ResidualVector().norm2();
    double err_x = 10000;
    double err_x0 = 10000;
    while (err>tol && err_x>tol && counter<1000)
    {
        CVector X0;
        if (trans == transformation::linear)
            X0 = GetContributionVector(false);
        else if (trans == transformation::softmax)
            X0 = GetContributionVectorSoftmax();

        err_p = err;
        CVector dx;

        if (trans == transformation::linear)
            dx = OneStepLevenBerg_Marquardt(lambda);
        else if (trans == transformation::softmax)
            dx = OneStepLevenBerg_Marquardt_softmax(lambda);

        if (dx.num == 0)
           lambda *= 5; 
        else
        {
            err_x = dx.norm2();
            if (counter == 0) err_x0 = err_x;
            CVector X = X0 - dx;
            if (trans == transformation::linear)
                SetContribution(X);
            else if (trans == transformation::softmax)
                SetContributionSoftmax(X);

            CVector V = ResidualVector();
            err = V.norm2();
            if (err < err_p * 0.8)
            {
                lambda /= 1.2;
            }
            else if (err > err_p)
            {
                lambda *= 1.2;
                if (trans == transformation::linear)
                    SetContribution(X0);
                else if (trans == transformation::softmax)
                    SetContributionSoftmax(X0);
                err = err_p;
            }
            if (rtw_)
            {
                rtw_->AppendPoint(counter, err);
                rtw_->SetXRange(0, counter);
                rtw_->SetProgress(1 - err_x / err_x0);
                
            }
            
        }
        counter++;
    }

    return false;
}


CVector SourceSinkData::PredictTarget(parameter_mode param_mode)
{
    CVector C = SourceMeanMatrix(param_mode)*GetContributionVector();
    for (unsigned int i=0; i<element_order_.size(); i++)
    {
        observation(i)->SetPredictedValue(C[i]);
    }
    return C;
}

CVector SourceSinkData::PredictTarget_Isotope(parameter_mode param_mode)
{
    CMatrix srcMatrixIso = SourceMeanMatrix_Isotopes(param_mode);
    CVector contribVect = GetContributionVector();
    CVector C = srcMatrixIso*contribVect;

    return C;
}

CVector SourceSinkData::PredictTarget_Isotope_delta(parameter_mode param_mode)
{
    CMatrix SourceMeanMat = SourceMeanMatrix(param_mode);
    CMatrix SourceMeanMat_Iso = SourceMeanMatrix_Isotopes(param_mode);
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
    return -LogLikelihood(parameter_estimation_mode);
}

double SourceSinkData::LogLikelihood(estimation_mode est_mode)
{
    double YLogLikelihood = 0;
    double CLogLikelihood = 0;
    double CLogLikelihood_Isotope = 0;
    if (est_mode != estimation_mode::only_contributions)
        YLogLikelihood = LogLikelihoodSourceElementalDistributions();
    double LogPrior = LogPriorContributions();
    if (est_mode != estimation_mode::source_elemental_profiles_based_on_source_data)
    {   CLogLikelihood = LogLikelihoodModelvsMeasured(est_mode);
        CLogLikelihood_Isotope = LogLikelihoodModelvsMeasured_Isotope(est_mode);
    }
    return YLogLikelihood + CLogLikelihood + CLogLikelihood_Isotope + LogPrior;
}

CMatrix SourceSinkData::SourceMeanMatrix(parameter_mode param_mode)
{
    CMatrix Y(element_order_.size(),numberofsourcesamplesets_);
    for (unsigned int element_counter=0; element_counter<element_order_.size(); element_counter++)
    {
        for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets_; source_group_counter++)
        {
            Elemental_Profile_Set *this_source_group = GetSampleSet(samplesetsorder_[source_group_counter]);
            if (param_mode==parameter_mode::based_on_fitted_distribution)
                Y[element_counter][source_group_counter] = this_source_group->GetEstimatedDistribution(element_order_[element_counter])->Mean();
            else
                Y[element_counter][source_group_counter] = this_source_group->GetEstimatedDistribution(element_order_[element_counter])->DataMean();
        }
    }
    return Y;
}

CMatrix SourceSinkData::SourceMeanMatrix_Isotopes(parameter_mode param_mode)
{
    CMatrix Y(isotope_order_.size(),numberofsourcesamplesets_);
    for (unsigned int isotope_counter=0; isotope_counter<isotope_order_.size(); isotope_counter++)
    {
        for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets_; source_group_counter++)
        {
            string isotope = isotope_order_[isotope_counter];
            string corresponding_element = element_information_[isotope].base_element;
            Elemental_Profile_Set *this_source_group = GetSampleSet(samplesetsorder_[source_group_counter]);

            if (param_mode==parameter_mode::based_on_fitted_distribution)
            {
                double mean_delta = this_source_group->GetEstimatedDistribution(isotope_order_[isotope_counter])->Mean();
                double standard_ratio = element_information_[isotope].standard_ratio;
                double corresponding_element_content = this_source_group->GetEstimatedDistribution(corresponding_element)->Mean();
                Y[isotope_counter][source_group_counter] = (mean_delta/1000.0+1.0)*standard_ratio*corresponding_element_content;

            }
            else
            {
                double mean_delta = this_source_group->GetEstimatedDistribution(isotope_order_[isotope_counter])->DataMean();
                double standard_ratio = element_information_[isotope].standard_ratio;
// Add a corresponding element check

                double corresponding_element_content = this_source_group->GetEstimatedDistribution(corresponding_element)->DataMean();
                Y[isotope_counter][source_group_counter] = (mean_delta/1000.0+1.0)*standard_ratio*corresponding_element_content;

            }
        }
    }
    return Y;
}

CVector SourceSinkData::GetContributionVector(bool full)
{
    if (full)
    {   CVector X(numberofsourcesamplesets_);
        for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets_; source_group_counter++)
        {
            Elemental_Profile_Set *this_source_group = GetSampleSet(samplesetsorder_[source_group_counter]);
            X[source_group_counter] = this_source_group->GetContribution();
        }
        return X;
    }
    else
    {   CVector X(numberofsourcesamplesets_-1);
        for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets_-1; source_group_counter++)
        {
            Elemental_Profile_Set *this_source_group = GetSampleSet(samplesetsorder_[source_group_counter]);
            X[source_group_counter] = this_source_group->GetContribution();
        }
        return X;
    }
}

CVector SourceSinkData::GetContributionVectorSoftmax()
{

   CVector X(numberofsourcesamplesets_);
    for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets_; source_group_counter++)
    {
        Elemental_Profile_Set *this_source_group = GetSampleSet(samplesetsorder_[source_group_counter]);
        X[source_group_counter] = this_source_group->GetContributionSoftmax(); 
    }
    return X;

}





void SourceSinkData::SetContribution(int i, double value)
{
    GetSampleSet(samplesetsorder_[i])->SetContribution(value);
    GetSampleSet(samplesetsorder_[samplesetsorder_.size()-1])->SetContribution(1-GetContributionVector(false).sum());
}

void SourceSinkData::SetContributionSoftmax(int i, double value)
{
    GetSampleSet(samplesetsorder_[i])->SetContributionSoftmax(value);
}

void SourceSinkData::SetContribution(const CVector &X)
{
    for (int i=0; i<X.num; i++)
    {
        SetContribution(i,X.vec[i]);
    }
    if (X.min()<0)
    {
        qDebug()<<"oops!";
    }
}

void SourceSinkData::SetContributionSoftmax(const CVector &X)
{
    double denominator = 0;
    for (int i=0; i<X.num; i++)
    {
        denominator += exp(X[i]);
    }
    for (int i=0; i<X.num; i++)
    {
        SetContributionSoftmax(i,X.at(i));
        SetContribution(i,exp(X.at(i))/denominator);
    }
    CVector XX = GetContributionVector();
    if (XX.min()<0)
    {
        qDebug()<<"oops!";
    }
}


Parameter* SourceSinkData::ElementalContent_mu(int element_iterator, int source_iterator)
{
    return &parameters_[size()-2+element_iterator*numberofsourcesamplesets_ +source_iterator];
}
Parameter* SourceSinkData::ElementalContent_sigma(int element_iterator, int source_iterator)
{
    return &parameters_[size()-2+element_iterator*numberofsourcesamplesets_ +source_iterator + numberofconstituents_*numberofsourcesamplesets_];
}

double SourceSinkData::ElementalContent_mu_value(int element_iterator, int source_iterator)
{
    return ElementalContent_mu(element_iterator,source_iterator)->Value();
}
double SourceSinkData::ElementalContent_sigma_value(int element_iterator, int source_iterator)
{
    return ElementalContent_sigma(element_iterator,source_iterator)->Value();
}

bool SourceSinkData::SetParameterValue(unsigned int i, double value)
{
    if (i<0 || i>parameters_.size())
        return false;

    int est_mode_key_1=1;
    int est_mode_key_2=1;
    if (parameter_estimation_mode==estimation_mode::only_contributions)
        est_mode_key_2 = 0;
    if (parameter_estimation_mode==estimation_mode::source_elemental_profiles_based_on_source_data)
        est_mode_key_1 = 0;

    parameters_[i].SetValue(value);
    //Contributions
    if (i<est_mode_key_1*(numberofsourcesamplesets_-1))
    {
        GetSampleSet(samplesetsorder_[i])->SetContribution(value);
        GetSampleSet(samplesetsorder_[numberofsourcesamplesets_-1])->SetContribution(1-GetContributionVector(false).sum());//+GetSampleSet(samplesetsorder_[numberofsourcesamplesets_-1])->GetContribution()
        return true;
    }
    //Element means
    else if (i<est_mode_key_1*(numberofsourcesamplesets_-1)+numberofconstituents_*numberofsourcesamplesets_ && parameter_estimation_mode==estimation_mode::elemental_profile_and_contribution)
    {
        int element_counter = (i-(numberofsourcesamplesets_-1))/numberofsourcesamplesets_;
        int group_counter = (i-(numberofsourcesamplesets_-1))%numberofsourcesamplesets_;
        GetElementDistribution(element_order_[element_counter],samplesetsorder_[group_counter])->SetEstimatedMu(value);
        return true;
    }
    //Isotopes means
    else if (i<est_mode_key_1*(numberofsourcesamplesets_-1)+(numberofconstituents_+numberofisotopes_)*numberofsourcesamplesets_ && parameter_estimation_mode==estimation_mode::elemental_profile_and_contribution)
    {
        int isotope_counter = (i-(numberofsourcesamplesets_-1)-numberofconstituents_*numberofsourcesamplesets_)/numberofsourcesamplesets_;
        int group_counter = (i-(numberofsourcesamplesets_-1-numberofconstituents_*numberofsourcesamplesets_))%numberofsourcesamplesets_;
        GetElementDistribution(isotope_order_[isotope_counter],samplesetsorder_[group_counter])->SetEstimatedMu(value);
        return true;
    }

    //Element standard deviations
    else if (i<est_mode_key_1*(numberofsourcesamplesets_-1)+(2*numberofconstituents_+numberofisotopes_)*numberofsourcesamplesets_ && parameter_estimation_mode==estimation_mode::elemental_profile_and_contribution)
    {
        if (value<0)
        {
            cout<<"stop!"<<std::endl;
        }
        int element_counter = (i-(numberofsourcesamplesets_-1)-(numberofconstituents_+numberofisotopes_)*numberofsourcesamplesets_)/numberofsourcesamplesets_;
        int group_counter = (i-(numberofsourcesamplesets_-1)-(numberofconstituents_+numberofisotopes_)*numberofsourcesamplesets_)%numberofsourcesamplesets_;
        GetElementDistribution(element_order_[element_counter],samplesetsorder_[group_counter])->SetEstimatedSigma(value);
        return true;
    }
    //Isotope standard deviations
    else if (i<est_mode_key_1*(numberofsourcesamplesets_-1)+(2*numberofconstituents_+2*numberofisotopes_)*numberofsourcesamplesets_ && parameter_estimation_mode==estimation_mode::elemental_profile_and_contribution)
    {
        if (value<0)
        {
            cout<<"stop!"<<std::endl;
        }
        int isotope_counter = (i-(numberofsourcesamplesets_-1)-(2*numberofconstituents_+numberofisotopes_)*numberofsourcesamplesets_)/numberofsourcesamplesets_;
        int group_counter = (i-(numberofsourcesamplesets_-1)-(2*numberofconstituents_+numberofisotopes_)*numberofsourcesamplesets_)%numberofsourcesamplesets_;
        GetElementDistribution(isotope_order_[isotope_counter],samplesetsorder_[group_counter])->SetEstimatedSigma(value);
        return true;
    }

    //Observed standard deviation
    else if (i==est_mode_key_1*(numberofsourcesamplesets_-1)+2*(numberofconstituents_+numberofisotopes_)*numberofsourcesamplesets_*est_mode_key_2)
    {
        if (value<0)
        {
            cout<<"stop!"<<std::endl;
        }
        error_stdev_ = value;
        return true;
    }
    //Observed isotope standard deviation
    else if (i==est_mode_key_1*(numberofsourcesamplesets_-1)+2*(numberofconstituents_+numberofisotopes_)*numberofsourcesamplesets_*est_mode_key_2+1)
    {
        if (value<0)
        {
            cout<<"stop!"<<std::endl;
        }
        error_stdev_isotope_ = value;
        return true;
    }

    return false;
}

double SourceSinkData::GetParameterValue(unsigned int i)
{
    return parameters_[i].Value();
}


bool SourceSinkData::SetParameterValue(const CVector &value)
{
    bool out = true;
    for (int i=0; i<value.num; i++)
    {
        out &=SetParameterValue(i,value[i]);
    }
    return out;
}

CVector SourceSinkData::GetParameterValue()
{
    CVector out(parameters_.size());
    for (unsigned int i=0; i<parameters_.size(); i++)
    {
        out[i]=parameters_[i].Value();
    }
    return out;
}

CVector SourceSinkData::Gradient(const CVector &value, const estimation_mode estmode)
{

    CVector out(value.num);
    CVector base_vector = value;
    SetParameterValue(value);
    double baseValue = LogLikelihood(estmode);
    for (int i=0; i<value.num; i++)
    {
        base_vector[i]+=epsilon_;
        SetParameterValue(base_vector);
        double loglikehood = LogLikelihood(estmode);
        out[i] = (loglikehood-baseValue)/epsilon_;
    }
    return out/out.norm2();
}

CVector SourceSinkData::GradientUpdate(const estimation_mode estmode)
{
    CVector X = GetParameterValue();
    double baseLikelihood = LogLikelihood(estmode);
    CVector dx = Gradient(X,estmode);
    CVector X_new1 = X+distance_coeff_*dx;
    SetParameterValue(X_new1);
    double newLikelihood1 = LogLikelihood(estmode);
    CVector X_new2 = X+2*distance_coeff_*dx;
    SetParameterValue(X_new2);
    double newLikelihood2 = LogLikelihood(estmode);
    qDebug()<<"Distance Coeff:" << distance_coeff_;
    if (distance_coeff_<10e-6)
        distance_coeff_=1;
    if (newLikelihood2>newLikelihood1 && newLikelihood2>baseLikelihood)
    {
        distance_coeff_*=2;
        return X_new2;
    }
    else if (newLikelihood1>newLikelihood2 && newLikelihood1>baseLikelihood)
    {
        SetParameterValue(X_new1);
        return X_new1;
    }
    else
    {
        int counter = 0;
        while (baseLikelihood>newLikelihood1 || counter<5)
        {
            distance_coeff_*=0.5;
            X_new1 = X+distance_coeff_*dx;
            SetParameterValue(X_new1);
            newLikelihood1 = LogLikelihood(estmode);
            counter++;
        }
        if (counter<5)
        {   SetParameterValue(X_new1);
            return X_new1;
        }
        else
        {
            SetParameterValue(X);
            return X;
        }
    }

}




vector<string> SourceSinkData::ElementsToBeUsedInCMB()
{
    numberofconstituents_ = 0;
    constituent_order_.clear();

    for (map<string, element_information>::iterator it = element_information_.begin(); it!=element_information_.end(); it++)
        if (it->second.Role == element_information::role::element)
        {   numberofconstituents_ ++;
            constituent_order_.push_back(it->first);
        }
    return constituent_order_;
}


vector<string> SourceSinkData::IsotopesToBeUsedInCMB()
{
    numberofisotopes_ = 0;
    isotope_order_.clear();

    for (map<string, element_information>::iterator it = element_information_.begin(); it!=element_information_.end(); it++)
        if (it->second.Role == element_information::role::isotope && it->second.include_in_analysis)
        {   numberofisotopes_ ++;
            isotope_order_.push_back(it->first);
        }
    return isotope_order_;
}

void SourceSinkData::PopulateConstituentOrders()
{
    numberofconstituents_ = 0;
    constituent_order_.clear();
    element_order_.clear();
    size_om_order_.clear();
    isotope_order_.clear();

    for (map<string, element_information>::iterator it = element_information_.begin(); it!=element_information_.end(); it++)
        {   numberofconstituents_ ++;
            constituent_order_.push_back(it->first);
        }
    for (map<string, element_information>::iterator it = element_information_.begin(); it!=element_information_.end(); it++)
        if (it->second.Role == element_information::role::element && it->second.include_in_analysis)
        {
            element_order_.push_back(it->first);
        }
    for (map<string, element_information>::iterator it = element_information_.begin(); it!=element_information_.end(); it++)
        if (it->second.Role == element_information::role::isotope && it->second.include_in_analysis)
        {
            isotope_order_.push_back(it->first);
        }
    for (map<string, element_information>::iterator it = element_information_.begin(); it!=element_information_.end(); it++)
        if (it->second.Role == element_information::role::particle_size)
        {
            size_om_order_.push_back(it->first);
        }
    for (map<string, element_information>::iterator it = element_information_.begin(); it!=element_information_.end(); it++)
        if (it->second.Role == element_information::role::organic_carbon)
        {
            size_om_order_.push_back(it->first);
        }

}

vector<string> SourceSinkData::SourceGroupNames()
{
    vector<string> sourcegroups;
    for (map<string,Elemental_Profile_Set>::iterator source_iterator = begin(); source_iterator!=end(); source_iterator++)
    {
        if (source_iterator->first!=target_group_)
            sourcegroups.push_back(source_iterator->first);
    }
    return sourcegroups;
}

bool SourceSinkData::SetSelectedTargetSample(const string &sample_name)
{
    if (GetSampleSet(GetTargetGroup())->GetProfile(sample_name))
    {
        selected_target_sample_ = sample_name;
        return true;
    }
    return false;
}
string SourceSinkData::SelectedTargetSample()
{
    return selected_target_sample_;
}

Elemental_Profile *SourceSinkData::GetElementalProfile(const string sample_name)
{

    for (map<string,Elemental_Profile_Set>::const_iterator group=begin(); group!=end(); group++ )
    {
        for (map<string,Elemental_Profile>::const_iterator sample=group->second.cbegin(); sample!=group->second.cend(); sample++)
            if (sample->first == sample_name)
                return GetSampleSet(group->first)->GetProfile(sample->first);
    }
    return nullptr;
}

ResultItem SourceSinkData::GetContribution()
{
    ResultItem result_cont;
    Contribution *contribution = new Contribution();
    for (int i=0; i<GetSourceOrder().size(); i++)
    {
        contribution->operator[](GetSourceOrder()[i]) = GetContributionVector()[i];
    }
    result_cont.SetName("Contributions");
    result_cont.SetResult(contribution);
    result_cont.SetType(result_type::contribution);

    return  result_cont;
}
ResultItem SourceSinkData::GetPredictedElementalProfile(parameter_mode param_mode)
{
    ResultItem result_modeled;

    Elemental_Profile *modeled_profile = new Elemental_Profile();
    CVector predicted_profile = PredictTarget(param_mode);
    vector<string> element_names = ElementOrder();
    for (int i=0; i<element_names.size(); i++)
    {
        modeled_profile->AppendElement(element_names[i],predicted_profile[i]);
    }
    result_modeled.SetName("Modeled Elemental Profile");
    result_modeled.SetResult(modeled_profile);
    result_modeled.SetType(result_type::predicted_concentration);
    return result_modeled;
}

CVector SourceSinkData::GetPredictedValues()
{
    CVector out(ObservationsCount());
    for (unsigned int i=0; i<ObservationsCount(); i++)
    {
        out[i] = observation(i)->PredictedValue();
    }
    return out;
}

ResultItem SourceSinkData::GetPredictedElementalProfile_Isotope(parameter_mode param_mode)
{
    ResultItem result_modeled;

    Elemental_Profile *modeled_profile = new Elemental_Profile();
    CVector predicted_profile = PredictTarget_Isotope_delta(param_mode);
    vector<string> isotope_names = IsotopeOrder();
    for (unsigned int i=0; i<isotope_names.size(); i++)
    {
        modeled_profile->AppendElement(isotope_names[i],predicted_profile[i]);
    }
    result_modeled.SetName("Modeled Elemental Profile for Isotopes");
    result_modeled.SetResult(modeled_profile);
    result_modeled.SetType(result_type::predicted_concentration);
    return result_modeled;
}

ResultItem SourceSinkData::GetObservedvsModeledElementalProfile(parameter_mode param_mode)
{
    ResultItem result_obs;

    Elemental_Profile* predicted = static_cast<Elemental_Profile*>(GetPredictedElementalProfile(param_mode).Result());
    Elemental_Profile* observed = static_cast<Elemental_Profile*>(GetObservedElementalProfile().Result());
    Elemental_Profile_Set* modeled_vs_observed = new Elemental_Profile_Set(); 
    modeled_vs_observed->AppendProfile("Observed", *observed);
    modeled_vs_observed->AppendProfile("Modeled", *predicted);
    
    result_obs.SetName("Observed vs Modeled Elemental Profile");
    result_obs.SetResult(modeled_vs_observed);
    result_obs.SetType(result_type::elemental_profile_set);
    return result_obs;
}

vector<ResultItem> SourceSinkData::GetMLRResults()
{
    vector<ResultItem> out;
    for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=end(); it++ )
    {
        ResultItem profile_result = it->second.GetRegressionsAsResult();
        profile_result.SetShowTable(true);
        profile_result.SetName("OM & Size MLR for " + it->first);
        out.push_back(profile_result);
    }
    return out;
}

ResultItem SourceSinkData::GetObservedvsModeledElementalProfile_Isotope(parameter_mode param_mode)
{
    ResultItem result_obs;

    Elemental_Profile* predicted = static_cast<Elemental_Profile*>(GetPredictedElementalProfile_Isotope(param_mode).Result());
    Elemental_Profile* observed = static_cast<Elemental_Profile*>(GetObservedElementalProfile_Isotope().Result());
    Elemental_Profile_Set* modeled_vs_observed = new Elemental_Profile_Set();
    modeled_vs_observed->AppendProfile("Observed", *observed);
    modeled_vs_observed->AppendProfile("Modeled", *predicted);

    result_obs.SetName("Observed vs Modeled Elemental Profile for Isotopes");
    result_obs.SetResult(modeled_vs_observed);
    result_obs.SetType(result_type::elemental_profile_set);
    return result_obs;
}

ResultItem SourceSinkData::GetObservedElementalProfile()
{
    ResultItem result_obs;

    Elemental_Profile* obs_profile = new Elemental_Profile();
    CVector observed_profile = ObservedDataforSelectedSample(selected_target_sample_);
    vector<string> element_names = ElementOrder();
    for (int i = 0; i < element_names.size(); i++)
    {
        obs_profile->AppendElement(element_names[i], observed_profile[i]);
    }
    CVector modeled_profile = ObservedDataforSelectedSample(selected_target_sample_);
    
    for (int i = 0; i < element_names.size(); i++)
    {
        obs_profile->AppendElement(element_names[i], observed_profile[i]);
    }
    result_obs.SetName("Observed Elemental Profile");
    result_obs.SetResult(obs_profile);
    result_obs.SetType(result_type::predicted_concentration);
    return result_obs;
}

ResultItem SourceSinkData::GetObservedElementalProfile_Isotope()
{
    ResultItem result_obs;

    Elemental_Profile* obs_profile = new Elemental_Profile();
    CVector observed_profile = ObservedDataforSelectedSample_Isotope_delta(selected_target_sample_);
    vector<string> isotope_names = IsotopeOrder();
    for (unsigned int i = 0; i < isotope_names.size(); i++)
    {
        obs_profile->AppendElement(isotope_names[i], observed_profile[i]);
    }

    result_obs.SetName("Observed Elemental Profile for Isotopes");
    result_obs.SetResult(obs_profile);
    result_obs.SetType(result_type::predicted_concentration);
    return result_obs;
}

ResultItem SourceSinkData::GetCalculatedElementMeans()
{
    Elemental_Profile_Set *profile_set = new Elemental_Profile_Set();
    for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=end(); it++ )
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
    vector<ResultItem> out;
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != end(); it++)
    {
        if (it->first != target_group_)
        {
            ResultItem result_item;
            result_item.SetName("Elemental Profiles for " + it->first);
            result_item.SetShowAsString(true);
            result_item.SetShowTable(true);
            result_item.SetShowGraph(true);
            result_item.SetType(result_type::elemental_profile_set);

            Elemental_Profile_Set* profile_set = new Elemental_Profile_Set();
            *profile_set = it->second;
            result_item.SetResult(profile_set);
            out.push_back(result_item);

        }
    }
    return out;
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
                element_profile.AppendElement(element_order_[element_counter], exp(mu+pow(sigma,2)/2));
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


bool SourceSinkData::WriteElementInformationToFile(QFile *file)
{
    file->write("***\n");
    file->write("Element Information\n");
    for (map<string,element_information>::iterator it = element_information_.begin(); it!=element_information_.end(); it++)
        file->write(QString::fromStdString(it->first).toUtf8()+ "\t" + Role(it->second.Role).toUtf8()+"\n");

    return true;
}

QJsonObject SourceSinkData::ElementInformationToJsonObject()
{
    QJsonObject json_object;
    
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

QJsonArray SourceSinkData::ToolsUsedToJsonObject()
{
    QJsonObject json_object;
    QJsonArray tools_used_json_array;
    for (list<string>::iterator it = tools_used.begin(); it != tools_used.end(); it++)
    {
        tools_used_json_array.append(QString::fromStdString(*it));
    }

    return tools_used_json_array;
}

QJsonObject SourceSinkData::OptionsToJsonObject()
{
    QJsonObject json_object;
    for (QMap<QString,double>::iterator it = options_.begin(); it != options_.end(); it++)
    {
        json_object[it.key()] = it.value();
    }

    return json_object;
}


void SourceSinkData::AddtoToolsUsed(const string &tool)
{
    if (!ToolsUsed(tool))
        tools_used.push_back(tool);
}


bool SourceSinkData::ReadToolsUsedFromJsonObject(const QJsonArray &jsonarray)
{
    foreach (const QJsonValue & value, jsonarray)
    {
        AddtoToolsUsed(value.toString().toStdString());
    }

    return true;
}


bool SourceSinkData::ReadElementInformationfromJsonObject(const QJsonObject &jsonobject)
{
    element_information_.clear();
    for(QString key: jsonobject.keys() ) {
        element_information elem_info;
        elem_info.Role = Role(jsonobject[key].toObject()["Role"].toString());
        elem_info.standard_ratio = jsonobject[key].toObject()["Standard Ratio"].toDouble();
        elem_info.base_element = jsonobject[key].toObject()["Base Element"].toString().toStdString();
        elem_info.include_in_analysis = jsonobject[key].toObject()["Include"].toBool();
        element_information_[key.toStdString()] = elem_info;
    }

    return true;
}

bool SourceSinkData::ReadElementDatafromJsonObject(const QJsonObject &jsonobject)
{
    clear();
    for(QString key: jsonobject.keys() ) {
        Elemental_Profile_Set elemental_profile_set;
        elemental_profile_set.ReadFromJsonObject(jsonobject[key].toObject());
        operator[](key.toStdString()) = elemental_profile_set;
    }

    return true;
}

bool SourceSinkData::ReadOptionsfromJsonObject(const QJsonObject &jsonobject)
{
    for(QString key: jsonobject.keys() ) {
        options_[key] = jsonobject[key].toDouble();
    }

    return true;
}

QJsonObject SourceSinkData::ElementDataToJsonObject()
{
    QJsonObject json_object;
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it != end(); it++)
    {
        json_object[QString::fromStdString(it->first)] = it->second.toJsonObject();
    }
    return json_object;
}


bool SourceSinkData::WriteDataToFile(QFile *file)
{
    file->write("***\n");
    file->write("Elemental Profiles\n");
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it!=end(); it++)
    {
        file->write("**\n");
        file->write(QString::fromStdString(it->first+"\n").toUtf8());
        it->second.writetofile(file);
    }
    return true;
}

bool SourceSinkData::WriteToFile(QFile *file)
{
    return true;
}

bool SourceSinkData::ReadFromFile(QFile *fil)
{
    Clear();
    QJsonObject jsondoc = QJsonDocument().fromJson(fil->readAll()).object();
    ReadElementDatafromJsonObject(jsondoc["Element Data"].toObject());
    ReadElementInformationfromJsonObject(jsondoc["Element Information"].toObject());
    ReadToolsUsedFromJsonObject(jsondoc["Tools used"].toArray());
    ReadOptionsfromJsonObject(jsondoc["Options"].toObject());
    target_group_ = jsondoc["Target Group"].toString().toStdString();
    return true;
}

QString SourceSinkData::Role(const element_information::role &rl)
{
    if (rl == element_information::role::do_not_include)
        return "DoNotInclude";
    else if (rl == element_information::role::element)
        return "Element";
    else if (rl == element_information::role::isotope)
        return "Isotope";
    else if (rl == element_information::role::particle_size)
        return "ParticleSize";
    else if (rl == element_information::role::organic_carbon)
        return "OM";
    return "DoNotInclude";
}

element_information::role SourceSinkData::Role(const QString &rl)
{
    if (rl == "DoNotInclude")
        return element_information::role::do_not_include;
    else if (rl == "Element")
        return element_information::role::element;
    else if (rl == "Isotope")
        return element_information::role::isotope;
    else if (rl == "ParticleSize")
        return element_information::role::particle_size;
    else if (rl == "OM")
        return element_information::role::organic_carbon;
    return element_information::role::do_not_include;
}

bool SourceSinkData::Perform_Regression_vs_om_size(const string &om, const string &d, regression_form form, const double& p_value_threshold)
{
    omconstituent_ = om;
    sizeconsituent_ = d;
    regression_p_value_threshold_ = p_value_threshold;
    for (map<string, Elemental_Profile_Set>::iterator it = begin(); it!=end(); it++)
    {
        it->second.SetRegressionModels(om,d,form,p_value_threshold);
    }
    return true;
}

void SourceSinkData::OutlierAnalysisForAll(const double &lowerthreshold, const double &upperthreshold)
{
    for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=end(); it++)
        if (it->first!=target_group_)
            it->second.DetectOutliers(lowerthreshold,upperthreshold);
}

DFA_result SourceSinkData::DiscriminantFunctionAnalysis()
{
    DFA_result out;
    out.F_test_P_value = CMBVector(size()-1);
    out.p_values = CMBVector(size()-1);
    out.wilkslambda = CMBVector(size()-1);

    int counter = 0;
    for (map<string, Elemental_Profile_Set>::iterator source = begin(); source!=end(); source++)
    {
        if (source->first!=target_group_)
        {
            DFA_result res = DiscriminantFunctionAnalysis(source->first);
            out.F_test_P_value[counter] = res.F_test_P_value[0];
            out.F_test_P_value.SetLabel(counter, source->first);
            out.p_values[counter] = res.p_values[0];
            out.p_values.SetLabel(counter, source->first);
            out.wilkslambda[counter] = res.wilkslambda[0];
            out.wilkslambda.SetLabel(counter, source->first);

            out.eigen_vectors.Append(source->first,res.eigen_vectors[source->first + " vs the rest"]);
            out.multi_projected.Append(source->first,res.projected);
            counter++;
        }

    }
    return out;
}


DFA_result SourceSinkData::DiscriminantFunctionAnalysis(const string &source1, const string &source2)
{
    SourceSinkData twoSources;
    twoSources.AppendSampleSet(source1,at(source1));
    twoSources.AppendSampleSet(source2,at(source2));
    twoSources.PopulateElementInformation(&element_information_);
    DFA_result out;
    //CMBVector eigen_vector = twoSources.DFA_weight_vector(source1, source2);
    CMBVector eigen_vector = twoSources.DFA_eigvector();
    if (eigen_vector.size()==0)
        return out;
    out.projected = twoSources.DFA_Projected(source1, source2);
    out.F_test_P_value = CMBVector(1); out.F_test_P_value[0] = out.projected.FTest_p_value();
    out.eigen_vectors.Append(source1 + " vs " +source2,eigen_vector);
    double p_value = twoSources.DFA_P_Value();
    out.p_values = CMBVector(1); out.p_values[0] = p_value;
    out.wilkslambda = CMBVector(1); out.wilkslambda[0] = twoSources.WilksLambda();
    return out;
}

DFA_result SourceSinkData::DiscriminantFunctionAnalysis(const string &source1)
{
    SourceSinkData twoSources;
    twoSources.AppendSampleSet(source1,at(source1));
    twoSources.AppendSampleSet("Others",TheRest(source1));
    twoSources.PopulateElementInformation(&element_information_);
    DFA_result out;
    //CMBVector eigen_vector = twoSources.DFA_weight_vector(source1, source2);
    CMBVector eigen_vector = twoSources.DFA_eigvector();
    out.projected = twoSources.DFA_Projected(source1, this);
    CMBVectorSet pairwiseProjected = twoSources.DFA_Projected(source1, "Others");
    out.F_test_P_value = CMBVector(1); out.F_test_P_value[0] = pairwiseProjected.FTest_p_value();
    out.eigen_vectors.Append(source1 + " vs the rest", eigen_vector);
    double p_value = twoSources.DFA_P_Value();
    out.p_values = CMBVector(1); out.p_values[0] = p_value;
    out.wilkslambda = CMBVector(1); out.wilkslambda[0] = twoSources.WilksLambda();
    return out;
}


int SourceSinkData::TotalNumberofSourceSamples() const
{
    int count = 0;
    for (map<string,Elemental_Profile_Set>::const_iterator source_group = cbegin(); source_group!=cend(); source_group++)
    {
        if (source_group->first != target_group_)
        {
            count += double(source_group->second.size());
        }
    }
    return count;
}

CMBVector SourceSinkData::DFATransformed(const CMBVector &eigenvector, const string &sourcegroup)
{
    CMBVector out(operator[](sourcegroup).size());
    int i=0;
    for (map<string,Elemental_Profile>::iterator sample = operator[](sourcegroup).begin(); sample != operator[](sourcegroup).end(); sample++)
    {
        out[i] = sample->second.CalculateDotProduct(eigenvector);
        out.SetLabel(i,sample->first);
        i++;
    }

    return out;
}

Elemental_Profile_Set SourceSinkData::TheRest(const string &source)
{
    Elemental_Profile_Set out;
    for (map<string,Elemental_Profile_Set>::iterator profile_set = begin(); profile_set!=end(); profile_set++)
    {
        if (profile_set->first!=source && profile_set->first!=target_group_)
            for (map<string, Elemental_Profile>::iterator profile=profile_set->second.begin(); profile!=profile_set->second.end(); profile++ )
            {
                out.AppendProfile(profile->first, profile->second);

            }
    }
    return out;
}


CMBVector SourceSinkData::BracketTest(const string &target_sample, bool correct_based_on_om_n_size)
{

    vector<string> element_names = GetElementNames();
    CMBVector out(element_names.size());
    CVector maxs(element_names.size());
    CVector mins(element_names.size());
    maxs.SetAllValues(1.0);
    mins.SetAllValues(1.0);
    SourceSinkData CopiedData = CreateCorrectedAndFilteredDataset(false, false, correct_based_on_om_n_size, target_sample);

    for (map<string,Elemental_Profile_Set>::iterator it = CopiedData.begin(); it!=CopiedData.end(); it++)
    {
        if (it->first != target_group_)
        {
            for (unsigned int i = 0; i < element_names.size(); i++)
            {
                out.SetLabel(i, element_names[i]);
                if (CopiedData.at(target_group_).GetProfile(target_sample)->at(element_names[i]) <= it->second.GetElementDistribution(element_names[i])->GetMaximum())
                {
                    maxs[i] = 0;
                }
                if (CopiedData.at(target_group_).GetProfile(target_sample)->at(element_names[i]) >= it->second.GetElementDistribution(element_names[i])->GetMinimum())
                {
                    mins[i] = 0;
                }

            }
        }
    }

    for (unsigned int i=0; i<element_names.size(); i++)
    {
        out[i] = max(maxs[i],mins[i]);
        if (maxs[i]>0.5)
            at(target_group_).GetProfile(target_sample)->AppendtoNotes(element_names[i] + " value is higher than the maximum of the sources");
        else if (mins[i]>0.5)
            at(target_group_).GetProfile(target_sample)->AppendtoNotes(element_names[i] + " value is lower than the minimum of the sources");
    }
    return out;
}


CMBMatrix SourceSinkData::BracketTest(bool correct_based_on_on_n_size, bool exclude_elements, bool exclude_samples)
{

    CMBMatrix out(at(target_group_).size(),CountElements(exclude_elements));
    int j=0;

    for (map<string,Elemental_Profile>::iterator sample = at(target_group_).begin(); sample != at(target_group_).end(); sample++ )
    {

        SourceSinkData copiedData;
        if (correct_based_on_on_n_size)
        {
            copiedData = CreateCorrectedAndFilteredDataset(exclude_samples, exclude_elements, true, sample->first);
        }
        else
            copiedData = CreateCorrectedAndFilteredDataset(exclude_samples,exclude_elements, false);

        vector<string> element_names = copiedData.GetElementNames();

        CMBVector bracket_vec = copiedData.BracketTest(sample->first,false);
        for (size_t i=0; i<element_names.size(); i++)
        {
            out[i][j] = bracket_vec.valueAt(i);
            out.SetRowLabel(i,element_names[i]);
        }
        out.SetColumnLabel(j,sample->first);
        j++;
    }

    return out;
}





SourceSinkData SourceSinkData::BoxCoxTransformed(bool calculateeigenvectors)
{
    CMBVector lambda_vals=OptimalBoxCoxParameters();
    SourceSinkData out(*this);
    for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=end(); it++)
    {
        if (it->first!=target_group_)
        {
            if (calculateeigenvectors)
                out[it->first] = it->second.ApplyBoxCoxTransform(&lambda_vals);
            else
                out[it->first] = it->second.ApplyBoxCoxTransform();
        }
    }
    out.PopulateElementDistributions();
    out.AssignAllDistributions();

    return out;
}

map<string,ConcentrationSet> SourceSinkData::ExtractConcentrationSet()
{
    map<string,ConcentrationSet> out;
    vector<string> element_names=GetElementNames();
    for (unsigned int i=0; i<element_names.size();i++)
    {   ConcentrationSet concset;
        for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=end(); it++)
        {
            if (it->first!=target_group_)
            {
                for (map<string,Elemental_Profile>::iterator sample=it->second.begin(); sample!=it->second.end(); sample++)
                    concset.AppendValue(sample->second[element_names[i]]);
            }
        }
        out[element_names[i]]=concset;

    }

    return out;
}

CMBVector SourceSinkData::OptimalBoxCoxParameters()
{
    map<string,ConcentrationSet> concset = ExtractConcentrationSet();
    vector<string> element_names=GetElementNames();
    CMBVector out(element_names.size());
    for (unsigned int i=0; i<element_names.size();i++)
    {
        out[i] = concset[element_names[i]].FindOptimalBoxCoxParameter(-5,5,10);

    }
    out.SetLabels(element_names);
    return out;
}

Elemental_Profile SourceSinkData::t_TestPValue(const string &source1, const string &source2, bool log)
{
    vector<string> element_names=GetElementNames();
    Elemental_Profile elemental_profile_item;
    for (unsigned int i=0; i<element_names.size();i++)
    {
        ConcentrationSet ConcentrationSet1 = *at(source1).GetElementDistribution(element_names[i]);
        ConcentrationSet ConcentrationSet2 = *at(source2).GetElementDistribution(element_names[i]);
        double std1, std2;
        double mean1, mean2;
        if (!log)
        {
            std1 = ConcentrationSet1.CalculateStdDev();
            std2 = ConcentrationSet2.CalculateStdDev();
            mean1 = ConcentrationSet1.CalculateMean();
            mean2 = ConcentrationSet2.CalculateMean();
        }
        else
        {
            std1 = ConcentrationSet1.CalculateStdDevLog();
            std2 = ConcentrationSet2.CalculateStdDevLog();
            mean1 = ConcentrationSet1.CalculateMeanLog();
            mean2 = ConcentrationSet2.CalculateMeanLog();
        }
        double t = (mean1-mean2)/sqrt(pow(std1,2)/ConcentrationSet1.size() + pow(std2,2)/ConcentrationSet2.size());
        double pvalueQ = gsl_cdf_tdist_Q (t, ConcentrationSet1.size() + ConcentrationSet2.size() - 2);
        pvalueQ = min(pvalueQ, 1.0-pvalueQ);
        double pvalueP = gsl_cdf_tdist_P (t, ConcentrationSet1.size() + ConcentrationSet2.size() - 2);
        pvalueP = min(pvalueP, 1.0-pvalueP);
        elemental_profile_item.AppendElement(element_names[i],pvalueQ+pvalueP);
    }
    return elemental_profile_item;

}

Elemental_Profile SourceSinkData::DifferentiationPower(const string &source1, const string &source2, bool log)
{
    vector<string> element_names=GetElementNames();
    Elemental_Profile elemental_profile_item;
    for (unsigned int i=0; i<element_names.size();i++)
    {
        ConcentrationSet ConcentrationSet1 = *at(source1).GetElementDistribution(element_names[i]);
        ConcentrationSet ConcentrationSet2 = *at(source2).GetElementDistribution(element_names[i]);
        double std1, std2;
        double mean1, mean2;
        if (!log)
        {
            std1 = ConcentrationSet1.CalculateStdDev();
            std2 = ConcentrationSet2.CalculateStdDev();
            mean1 = ConcentrationSet1.CalculateMean();
            mean2 = ConcentrationSet2.CalculateMean();
        }
        else
        {
            std1 = ConcentrationSet1.CalculateStdDevLog();
            std2 = ConcentrationSet2.CalculateStdDevLog();
            mean1 = ConcentrationSet1.CalculateMeanLog();
            mean2 = ConcentrationSet2.CalculateMeanLog();
        }
        elemental_profile_item.AppendElement(element_names[i],2*(fabs(mean1-mean2)/(std1+std2)));
    }
    return elemental_profile_item;

}


Elemental_Profile_Set SourceSinkData::DifferentiationPower(bool log, bool include_target)
{

    Elemental_Profile_Set out;
    for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=prev(end()); it++)
    {
        if (include_target || it->first!=target_group_)
        for (map<string,Elemental_Profile_Set>::iterator it2=next(it); it2!=end(); it2++)
        {
            if (include_target || it2->first!=target_group_)
            {   Elemental_Profile elem_prof = DifferentiationPower(it->first, it2->first, log);
                out.AppendProfile(it->first + " and " + it2->first, elem_prof);
            }
        }
    }
    return out;
}

Elemental_Profile SourceSinkData::DifferentiationPower_Percentage(const string &source1, const string &source2)
{
    vector<string> element_names=GetElementNames();
    Elemental_Profile elemental_profile_item;
    for (unsigned int i=0; i<element_names.size();i++)
    {
        ConcentrationSet ConcentrationSet1 = *at(source1).GetElementDistribution(element_names[i]);
        ConcentrationSet ConcentrationSet2 = *at(source2).GetElementDistribution(element_names[i]);
        ConcentrationSet ConcentrationSetAll = ConcentrationSet1;
        ConcentrationSetAll.AppendSet(ConcentrationSet2);
        vector<unsigned int> Rank = ConcentrationSetAll.CalculateRanks();
        int Set1BelowLimitCount = aquiutils::CountLessThan(Rank,ConcentrationSet1.size(),ConcentrationSet1.size(),false);
        int Set1AboveLimitCount = aquiutils::CountGreaterThan(Rank,ConcentrationSet1.size(),ConcentrationSet1.size(),false);
        int Set2BelowLimitCount = aquiutils::CountLessThan(Rank,ConcentrationSet1.size(),ConcentrationSet1.size(),true);
        int Set2AboveLimitCount = aquiutils::CountGreaterThan(Rank,ConcentrationSet1.size(),ConcentrationSet1.size(),true);
        double set1fraction = double(Set1BelowLimitCount+Set2AboveLimitCount)/double(ConcentrationSetAll.size());
        double set2fraction = double(Set1AboveLimitCount+Set2BelowLimitCount)/double(ConcentrationSetAll.size());
        elemental_profile_item.AppendElement(element_names[i],max(set1fraction,set2fraction));


    }
    return elemental_profile_item;

}

Elemental_Profile_Set SourceSinkData::DifferentiationPower_Percentage(bool include_target)
{
    Elemental_Profile_Set out;

    for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=prev(end()); it++)
    {
        if (include_target || it->first!=target_group_)
        for (map<string,Elemental_Profile_Set>::iterator it2=next(it); it2!=end(); it2++)
        {
            if (include_target || it2->first!=target_group_)
            {   Elemental_Profile elem_prof = DifferentiationPower_Percentage(it->first, it2->first);
                out.AppendProfile(it->first + " and " + it2->first, elem_prof);
            }
        }
    }
    return out;

}

Elemental_Profile_Set SourceSinkData::DifferentiationPower_P_value(bool include_target)
{
    Elemental_Profile_Set out;

    for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=prev(end()); it++)
    {
        if (include_target || it->first!=target_group_)
        for (map<string,Elemental_Profile_Set>::iterator it2=next(it); it2!=end(); it2++)
        {
            if (include_target || it2->first!=target_group_)
            {   Elemental_Profile elem_prof = t_TestPValue(it->first, it2->first,false);
                out.AppendProfile(it->first + " and " + it2->first, elem_prof);
            }
        }
    }
    return out;

}

vector<string> SourceSinkData::NegativeValueCheck()
{
    vector<string> out;
    PopulateConstituentOrders();
    for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=prev(end()); it++)
    {
        vector<string> NegativeItems = it->second.CheckForNegativeValues(element_order_);
        for (unsigned int i=0; i<NegativeItems.size(); i++ )
        {
            out.push_back("There are zero or negative values for element '" + NegativeItems[i] + "' in sample group '" + it->first +"'");
        }
    }
    return out;
}

void SourceSinkData::IncludeExcludeAllElements(bool value)
{
    for(map<string, element_information>::iterator it = element_information_.begin(); it!=element_information_.end(); it++)
    {
        it->second.include_in_analysis = value;
    }
}

double SourceSinkData::GrandMean(const string &element, bool logtransformed)
{
    double sum=0;
    double count=0;
    for (map<string,Elemental_Profile_Set>::iterator it=begin(); it!=prev(end()); it++)
    {
        if (it->first!=target_group_)
        {   if (!logtransformed)
            {   sum+=it->second.GetElementDistribution(element)->CalculateMean()*it->second.GetElementDistribution(element)->size();
                count += it->second.GetElementDistribution(element)->size();
            }
            else
            {   sum+=it->second.GetElementDistribution(element)->CalculateMeanLog()*it->second.GetElementDistribution(element)->size();
                count += it->second.GetElementDistribution(element)->size();
            }
        }
    }
    return sum/count;
}

Elemental_Profile_Set SourceSinkData::LumpAllProfileSets()
{
    Elemental_Profile_Set out;
    for (map<string,Elemental_Profile_Set>::const_iterator it=begin(); it!=prev(end()); it++)
    {
        if (it->first!=target_group_)
        {
            out.AppendProfiles(it->second,nullptr);
        }
    }
    out.UpdateElementDistributions();
    return out;
}

CMBVector SourceSinkData::ANOVA(bool logtransformed)
{
    vector<string> element_names = GetElementNames();
    CMBVector p_values(element_names.size());
    p_values.SetLabels(element_names);
    for (unsigned int i=0; i<element_names.size(); i++)
    {
        ANOVA_info anova = ANOVA(element_names[i],logtransformed);
        p_values[i] = anova.p_value;
    }
    return p_values;
}

ANOVA_info SourceSinkData::ANOVA(const string &element, bool logtransformed)
{
    ANOVA_info anova;
    Elemental_Profile_Set All_ProfileSets = LumpAllProfileSets();
    if (!logtransformed)
    {   anova.SST = All_ProfileSets.GetElementDistribution(element)->CalculateSSE();
        double sum=0;
        double sum_w=0;
        for (map<string,Elemental_Profile_Set>::iterator source_group = begin(); source_group!=end(); source_group++ )
        {
            if (source_group->first != target_group_)
            {   sum += pow(source_group->second.GetElementDistribution(element)->CalculateMean()-All_ProfileSets.GetElementDistribution(element)->CalculateMean(), 2) * source_group->second.GetElementDistribution(element)->size();
                sum_w += source_group->second.GetElementDistribution(element)->CalculateSSE();
            }
        }
        anova.SSB = sum;
        anova.SSW = sum_w;
    }
    else
    {   anova.SST = pow(All_ProfileSets.GetElementDistribution(element)->CalculateStdDevLog(),2)*(All_ProfileSets.GetElementDistribution(element)->size()-1);
        double sum=0;
        double sum_w=0;
        for (map<string,Elemental_Profile_Set>::iterator source_group = begin(); source_group!=end(); source_group++ )
        {
            if (source_group->first != target_group_)
            {   sum += pow(source_group->second.GetElementDistribution(element)->CalculateMeanLog()-All_ProfileSets.GetElementDistribution(element)->CalculateMeanLog(),2)*source_group->second.GetElementDistribution(element)->size();
                sum_w += source_group->second.GetElementDistribution(element)->CalculateSSELog();
            }
        }
        anova.SSB = sum;
        anova.SSW = sum_w;
    }
    anova.MSB = anova.SSB/(double(this->size()-2.0));
    anova.MSW = anova.SSW/(double(All_ProfileSets.GetElementDistribution(element)->size())-(this->size()-1));
    anova.F = anova.MSB/anova.MSW;
    anova.p_value = gsl_cdf_fdist_Q (anova.F, this->size()-2, All_ProfileSets.size());
    return anova;

}

void SourceSinkData::IncludeExcludeElementsBasedOn(const vector<string> elements)
{

    for (map<string,element_information>::iterator element = element_information_.begin(); element!=element_information_.end(); element++)
    {
        element->second.include_in_analysis=false;
    }
    for (unsigned int i=0; i<elements.size(); i++)
    {
        element_information_[elements[i]].include_in_analysis=true;
    }
}

SourceSinkData SourceSinkData::RandomlyEliminateSourceSamples(const double &percentage)
{
    SourceSinkData out;
    vector<string> samplestobeeliminated = RandomlypickSamples(percentage/100.0);
    for (map<string, Elemental_Profile_Set>::const_iterator it=cbegin(); it!=cend(); it++)
    {
        if (it->first!=target_group_)
        {
            out[it->first] = it->second.EliminateSamples(samplestobeeliminated,&element_information_);
        }
        else
            out[it->first] = it->second;
    }
    out.omconstituent_ = omconstituent_;
    out.sizeconsituent_ = sizeconsituent_;
    out.target_group_ = target_group_;
    out.PopulateElementInformation(&element_information_);
    out.PopulateElementDistributions();
    out.AssignAllDistributions();
    return out;
}


Elemental_Profile SourceSinkData::Sample(const string &samplename) const
{
    for (map<string, Elemental_Profile_Set>::const_iterator it=cbegin(); it!=cend(); it++)
    {
        if (it->second.count(samplename)==1)
        {
            return it->second.at(samplename);
        }
    }
    return Elemental_Profile();
}

SourceSinkData SourceSinkData::ReplaceSourceAsTarget(const string &sourcesamplename) const
{
    SourceSinkData out = *this;
    Elemental_Profile target = Sample(sourcesamplename);
    Elemental_Profile_Set targetgroup = Elemental_Profile_Set();
    targetgroup.AppendProfile(sourcesamplename,target);
    out[target_group_] = targetgroup;
    out.omconstituent_ = omconstituent_;
    out.sizeconsituent_ = sizeconsituent_;
    out.target_group_ = target_group_;
    out.PopulateElementInformation(&element_information_);
    out.PopulateElementDistributions();
    out.AssignAllDistributions();
    return out;
}

CMBTimeSeriesSet SourceSinkData::BootStrap(const double &percentage, unsigned int number_of_samples, string target_sample, bool softmax_transformation)
{
    CMBTimeSeriesSet result(numberofsourcesamplesets_);

    for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets_; source_group_counter++)
        result.setname(source_group_counter, samplesetsorder_[source_group_counter]);
    for (unsigned int i=0; i<number_of_samples; i++)
    {
        SourceSinkData bootstrappeddata = RandomlyEliminateSourceSamples(percentage);
        bootstrappeddata.InitializeParametersAndObservations(target_sample);
        if (rtw_)
            rtw_->SetProgress(double(i)/double(number_of_samples));

        if (softmax_transformation)
            bootstrappeddata.SolveLevenBerg_Marquardt(transformation::softmax);
        else
            bootstrappeddata.SolveLevenBerg_Marquardt(transformation::linear);

        result.append(i,bootstrappeddata.GetContributionVector().vec);
    }
    return result;
}

bool SourceSinkData::BootStrap(Results *res, const double &percentage, unsigned int number_of_samples, string target_sample, bool softmax_transformation)
{
    CMBTimeSeriesSet* contributions = new CMBTimeSeriesSet(numberofsourcesamplesets_);

    for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets_; source_group_counter++)
        contributions->setname(source_group_counter, samplesetsorder_[source_group_counter]);
    for (unsigned int i=0; i<number_of_samples; i++)
    {
        SourceSinkData bootstrappeddata = RandomlyEliminateSourceSamples(percentage);
        bootstrappeddata.InitializeParametersAndObservations(target_sample);
        if (rtw_)
            rtw_->SetProgress(double(i)/double(number_of_samples));

        if (softmax_transformation)
            bootstrappeddata.SolveLevenBerg_Marquardt(transformation::softmax);
        else
            bootstrappeddata.SolveLevenBerg_Marquardt(transformation::linear);

        contributions->append(i,bootstrappeddata.GetContributionVector().vec);
    }
    ResultItem contributions_result_item;
    res->SetName("Error Analysis for target sample'" + target_sample +"'");
    contributions_result_item.SetName("Error Analysis");
    contributions_result_item.SetResult(contributions);
    contributions_result_item.SetType(result_type::stacked_bar_chart);
    contributions_result_item.SetShowAsString(false);
    contributions_result_item.SetShowTable(true);
    if (number_of_samples<101)
        contributions_result_item.SetShowGraph(true);
    else
        contributions_result_item.SetShowGraph(false);
    contributions_result_item.SetYLimit(_range::high, 1);
    contributions_result_item.SetXAxisMode(xaxis_mode::counter);
    contributions_result_item.setYAxisTitle("Contribution");
    contributions_result_item.setXAxisTitle("Sample");
    contributions_result_item.SetYLimit(_range::low, 0);
    res->Append(contributions_result_item);

    CMBTimeSeriesSet *dists = new CMBTimeSeriesSet();
    *dists = contributions->distribution(100,0,0);
    ResultItem distribution_res_item;
    distribution_res_item.SetName("Posterior Distributions");
    distribution_res_item.SetShowAsString(false);
    distribution_res_item.SetShowTable(true);
    distribution_res_item.SetType(result_type::distribution);
    distribution_res_item.SetResult(dists);
    res->Append(distribution_res_item);

//Posterior contribution 95% intervals
    RangeSet *contribution_credible_intervals = new RangeSet();
    for (unsigned int i=0; i<GetSourceOrder().size(); i++)
    {
        Range range;
        double percentile_low = contributions->at(i).percentile(0.025);
        double percentile_high = contributions->at(i).percentile(0.975);
        double mean = contributions->at(i).mean();
        double median = contributions->at(i).percentile(0.5);
        range.Set(_range::low,percentile_low);
        range.Set(_range::high,percentile_high);
        range.SetMean(mean);
        range.SetMedian(median);
        contribution_credible_intervals->operator[](contributions->getSeriesName(i)) = range;
    }
    ResultItem contribution_credible_intervals_result_item;

    contribution_credible_intervals_result_item.SetName("Source Contribution Credible Intervals");
    contribution_credible_intervals_result_item.SetShowAsString(true);
    contribution_credible_intervals_result_item.SetShowTable(true);
    contribution_credible_intervals_result_item.SetType(result_type::rangeset);
    contribution_credible_intervals_result_item.SetResult(contribution_credible_intervals);
    contribution_credible_intervals_result_item.SetYAxisMode(yaxis_mode::normal);
    contribution_credible_intervals_result_item.SetYLimit(_range::high,1.0);
    contribution_credible_intervals_result_item.SetYLimit(_range::low,0);
    res->Append(contribution_credible_intervals_result_item);


    return true;

}

CMBTimeSeriesSet SourceSinkData::VerifySource(const string &sourcegroup, bool softmax_transformation, bool sizeandorganiccorrect)
{
    InitializeParametersAndObservations(GetSampleSet(target_group_)->begin()->first);
    CMBTimeSeriesSet result(numberofsourcesamplesets_);

    for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets_; source_group_counter++)
        result.setname(source_group_counter, samplesetsorder_[source_group_counter]);
    int counter = 0;
    for (map<string,Elemental_Profile>::iterator sample = at(sourcegroup).begin(); sample!=at(sourcegroup).end(); sample++)
    {

        SourceSinkData bootstrappeddata = ReplaceSourceAsTarget(sample->first);
        SourceSinkData correctedData = bootstrappeddata.CreateCorrectedDataset(sample->first,sizeandorganiccorrect,bootstrappeddata.GetElementInformation());
        correctedData.SetProgressWindow(rtw_);
        vector<string> negative_elements = correctedData.NegativeValueCheck();

        if (negative_elements.size()==0)
        {
            bootstrappeddata.InitializeParametersAndObservations(sample->first);
            if (rtw_)
                rtw_->SetProgress(double(counter)/double(at(sourcegroup).size()));

            if (softmax_transformation)
                bootstrappeddata.SolveLevenBerg_Marquardt(transformation::softmax);
            else
                bootstrappeddata.SolveLevenBerg_Marquardt(transformation::linear);

            result.append(counter,bootstrappeddata.GetContributionVector().vec);
            result.SetLabel(counter,sample->first);
            counter++;
        }
        else
        {
            for (int i=0; i<negative_elements.size(); i++)
            {
                qDebug()<<QString::fromStdString(negative_elements[i]);
            }
        }


    }
    return result;
}

CMBTimeSeriesSet SourceSinkData::LM_Batch(transformation transform, bool om_size_correction, map<string,vector<string>> &negative_elements)
{

    InitializeParametersAndObservations(GetSampleSet(target_group_)->begin()->first);
    CMBTimeSeriesSet result(numberofsourcesamplesets_);

    for (unsigned int source_group_counter=0; source_group_counter<numberofsourcesamplesets_; source_group_counter++)
        result.setname(source_group_counter, samplesetsorder_[source_group_counter]);
    int counter = 0;
    for (map<string,Elemental_Profile>::iterator sample = at(target_group_).begin(); sample!=at(target_group_).end(); sample++)
    {
        if (sample->first != "")
        {   SourceSinkData correctedData = CreateCorrectedDataset(sample->first,om_size_correction,GetElementInformation());
            negative_elements[sample->first] = correctedData.NegativeValueCheck();
            if (negative_elements[sample->first].size()==0)
            {   correctedData.InitializeParametersAndObservations(sample->first);
                if (rtw_)
                {   rtw_->SetProgress(double(counter)/double(at(target_group_).size()));
                    rtw_->SetLabel(QString::fromStdString(sample->first));
                }

                correctedData.SolveLevenBerg_Marquardt(transform);

                result.append(counter,correctedData.GetContributionVector().vec);
                result.SetLabel(counter,sample->first);
                counter++;
            }
        }

    }
    rtw_->SetProgress(1);
    return result;
}



vector<string> SourceSinkData::AllSourceSampleNames() const
{
    vector<string> out;
    for (map<string, Elemental_Profile_Set>::const_iterator profile_set=cbegin(); profile_set!=cend(); profile_set++)
    {
        if (profile_set->first!=target_group_)
        {
            for (map<string,Elemental_Profile>::const_iterator profile = profile_set->second.cbegin(); profile!=profile_set->second.cend(); profile++)
                out.push_back(profile->first);
        }
    }
    return out;


}
vector<string> SourceSinkData::RandomlypickSamples(const double &percentage) const
{
    vector<string> allsamples = AllSourceSampleNames();
    vector<string> out;
    for (unsigned int i=0; i<allsamples.size(); i++)
    {
        if (GADistribution::GetRndUniF(0,1)<percentage)
        {
            out.push_back(allsamples[i]);
        }
    }
    return out;
}

Results SourceSinkData::MCMC(const string &sample, map<string,string> arguments, CMCMC<SourceSinkData> *mcmc, ProgressWindow *rtw_, const string &workingfolder)
{
    Results results;
    results.SetName("MCMC results for '" +sample + "'");
    ResultItem MCMC_samples;
    MCMC_samples.SetShowAsString(false);
    MCMC_samples.SetType(result_type::mcmc_samples);
    MCMC_samples.SetName("MCMC samples");
    CMBTimeSeriesSet *samples = new CMBTimeSeriesSet();

    bool organicnsizecorrection;
    if (arguments["Apply size and organic matter correction"]=="true")
    {
        organicnsizecorrection = true;
    }
    else
        organicnsizecorrection = false;

    SourceSinkData correctedData = CreateCorrectedDataset(sample,organicnsizecorrection, GetElementInformation());
    vector<string> negative_elements = correctedData.NegativeValueCheck();
    if (negative_elements.size()>0)
    {
        results.SetError("Negative elemental content in ");
        for (unsigned int i=0; i<negative_elements.size(); i++)
        {
            if (i==0)
                results.AppendError(negative_elements[i]);
            else
                results.AppendError("," + negative_elements[i]);
        }
        return results;

    }

    mcmc->Model = &correctedData;

    rtw_->SetTitle("Acceptance Rate",0);
    rtw_->SetTitle("Purturbation Factor",1);
    rtw_->SetTitle("Log posterior value",2);
    rtw_->SetYAxisTitle("Acceptance Rate",0);
    rtw_->SetYAxisTitle("Purturbation Factor",1);
    rtw_->SetYAxisTitle("Log posterior value",2);
    rtw_->show();
// Samples
    qDebug()<<2;
    correctedData.InitializeParametersAndObservations(sample);
    mcmc->SetProperty("number_of_samples",arguments["Number of samples"]);
    mcmc->SetProperty("number_of_chains",arguments["Number of chains"]);
    mcmc->SetProperty("number_of_burnout_samples",arguments["Samples to be discarded (burnout)"]);
    mcmc->SetProperty("dissolve_chains",arguments["Dissolve Chains"]);
    qDebug()<<3;
    mcmc->initialize(samples,true);
    string folderpath;
    if (!QString::fromStdString(arguments["Samples File Name"]).contains("/"))
        folderpath = workingfolder+"/";
    qDebug()<<4;
    mcmc->step(QString::fromStdString(arguments["Number of chains"]).toInt(), QString::fromStdString(arguments["Number of samples"]).toInt(), folderpath + arguments["Samples File Name"], samples, rtw_);
    qDebug()<<"Outside MCMC";
    vector<string> SourceGroupNames = correctedData.SourceGroupNames();
    qDebug()<<"Appending last contributions";
    samples->AppendLastContribution(SourceGroupNames.size()-1,SourceGroupNames[SourceGroupNames.size()-1]+"_contribution");
    MCMC_samples.SetResult(samples);
    results.Append(MCMC_samples);
// Posterior distributions
    qDebug()<<"Posterior distributions";
    ResultItem distribution_res_item;
    CMBTimeSeriesSet *dists = new CMBTimeSeriesSet();
    *dists = samples->distribution(100,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
    distribution_res_item.SetName("Posterior Distributions");
    distribution_res_item.SetShowAsString(false);
    distribution_res_item.SetShowTable(true);
    distribution_res_item.SetType(result_type::distribution);
    distribution_res_item.SetResult(dists);
    results.Append(distribution_res_item);
    qDebug()<<"Posterior distributions 95%";
//Posterior contribution 95% intervals
    RangeSet *contribution_credible_intervals = new RangeSet();
    for (unsigned int i=0; i<correctedData.GetSourceOrder().size(); i++)
    {
        Range range;
        double percentile_low = samples->at(i).percentile(0.025,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
        double percentile_high = samples->at(i).percentile(0.975,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
        double mean = samples->at(i).mean(QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
        double median = samples->at(i).percentile(0.5,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
        range.Set(_range::low,percentile_low);
        range.Set(_range::high,percentile_high);
        range.SetMean(mean);
        range.SetMedian(median);
        contribution_credible_intervals->operator[](samples->getSeriesName(i)) = range;
    }
    ResultItem contribution_credible_intervals_result_item;

    contribution_credible_intervals_result_item.SetName("Source Contribution Credible Intervals");
    contribution_credible_intervals_result_item.SetShowAsString(true);
    contribution_credible_intervals_result_item.SetShowTable(true);
    contribution_credible_intervals_result_item.SetType(result_type::rangeset);
    contribution_credible_intervals_result_item.SetResult(contribution_credible_intervals);
    contribution_credible_intervals_result_item.SetYAxisMode(yaxis_mode::log);
    contribution_credible_intervals_result_item.SetYLimit(_range::high,1.0);
    results.Append(contribution_credible_intervals_result_item);
    qDebug()<<"Predicted distributions";
// Predicted 95% posterior distributions
    CMBTimeSeriesSet predicted_samples = mcmc->predicted;
    CMBTimeSeriesSet predicted_samples_elems;
    vector<string> ConstituentNames = correctedData.ElementOrder();
    vector<string> IsotopeNames = correctedData.IsotopeOrder();
    vector<string> AllNames = ConstituentNames;

    AllNames.insert(AllNames.end(),IsotopeNames.begin(), IsotopeNames.end());

    for (int i=0; i<predicted_samples.size(); i++)
        predicted_samples.setname(i,AllNames[i]);



    for (int i=0; i<predicted_samples.size(); i++)
    {
        if (correctedData.GetElementInformation(predicted_samples.getSeriesName(i))->Role==element_information::role::element)
            predicted_samples_elems.append(predicted_samples[i],predicted_samples.getSeriesName(i));
    }
    ResultItem predicted_distribution_res_item;
    CMBTimeSeriesSet *predicted_dists_elems = new CMBTimeSeriesSet();

    *predicted_dists_elems = predicted_samples_elems.distribution(100,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
    for (int i=0; i<predicted_samples.size(); i++)
        predicted_dists_elems->SetObservedValue(i,correctedData.observation(i)->Value());

    predicted_distribution_res_item.SetName("Posterior Predicted Constituents");
    predicted_distribution_res_item.SetShowAsString(false);
    predicted_distribution_res_item.SetShowTable(true);
    predicted_distribution_res_item.SetType(result_type::distribution_with_observed);
    predicted_distribution_res_item.SetResult(predicted_dists_elems);
    results.Append(predicted_distribution_res_item);
    qDebug()<<"Predicted distributions 95%";
//predicted 95% credible intervals

    RangeSet *predicted_credible_intervals = new RangeSet();
    vector<double> percentile_low = predicted_samples.percentile(0.025,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
    vector<double> percentile_high = predicted_samples.percentile(0.975,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
    vector<double> mean = predicted_samples.mean(QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
    vector<double> median = predicted_samples.percentile(0.5,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
    for (int i=0; i<predicted_dists_elems->size(); i++)
    {
        Range range;
        range.Set(_range::low,percentile_low[i]);
        range.Set(_range::high,percentile_high[i]);
        range.SetMean(mean[i]);
        range.SetMedian(median[i]);
        predicted_credible_intervals->operator[](predicted_dists_elems->getSeriesName(i)) = range;
        predicted_credible_intervals->operator[](predicted_dists_elems->getSeriesName(i)).SetValue(correctedData.observation(i)->Value());
    }
    ResultItem predicted_credible_intervals_result_item;
    predicted_credible_intervals_result_item.SetName("Predicted Samples Credible Intervals");
    predicted_credible_intervals_result_item.SetShowAsString(true);
    predicted_credible_intervals_result_item.SetShowTable(true);
    predicted_credible_intervals_result_item.SetType(result_type::rangeset_with_observed);
    predicted_credible_intervals_result_item.SetResult(predicted_credible_intervals);
    predicted_credible_intervals_result_item.SetYAxisMode(yaxis_mode::log);
    results.Append(predicted_credible_intervals_result_item);
    qDebug()<<"Predicted isotope distributions";
    // Predicted 95% posterior distributions for isotopes
    CMBTimeSeriesSet predicted_samples_isotopes;

    for (int i=0; i<predicted_samples.size(); i++)
    {
        if (correctedData.GetElementInformation(predicted_samples.getSeriesName(i))->Role==element_information::role::isotope)
            predicted_samples_isotopes.append(predicted_samples[i],predicted_samples.getSeriesName(i));
    }
    ResultItem predicted_distribution_iso_res_item;
    CMBTimeSeriesSet *predicted_dists_isotopes = new CMBTimeSeriesSet();

    *predicted_dists_isotopes = predicted_samples_isotopes.distribution(100,QString::fromStdString(arguments["Samples to be discarded (burnout)"]).toInt());
    for (int i=0; i<predicted_samples_isotopes.size(); i++)
        predicted_dists_isotopes->SetObservedValue(i,correctedData.observation(i+ConstituentNames.size())->Value());

    predicted_distribution_iso_res_item.SetName("Posterior Predicted Isotopes");
    predicted_distribution_iso_res_item.SetShowAsString(false);
    predicted_distribution_iso_res_item.SetShowTable(true);
    predicted_distribution_iso_res_item.SetType(result_type::distribution_with_observed);
    predicted_distribution_iso_res_item.SetResult(predicted_dists_isotopes);
    results.Append(predicted_distribution_iso_res_item);
    qDebug()<<"Predicted isotope distributions 95%";
    //predicted 95% credible intervals for isotopes

    RangeSet *predicted_credible_intervals_isotopes = new RangeSet();

    for (int i=0; i<predicted_dists_isotopes->size(); i++)
    {
        Range range;
        range.Set(_range::low,percentile_low[i+correctedData.ElementOrder().size()]);
        range.Set(_range::high,percentile_high[i+correctedData.ElementOrder().size()]);
        range.SetMean(mean[i+correctedData.ElementOrder().size()]);
        range.SetMedian(median[i+correctedData.ElementOrder().size()]);
        predicted_credible_intervals_isotopes->operator[](predicted_dists_isotopes->getSeriesName(i)) = range;
        predicted_credible_intervals_isotopes->operator[](predicted_dists_isotopes->getSeriesName(i)).SetValue(correctedData.observation(i+correctedData.ElementOrder().size())->Value());
    }
    ResultItem predicted_credible_intervals_isotope_result_item;
    predicted_credible_intervals_isotope_result_item.SetName("Predicted Samples Credible Intervals for Isotopes");
    predicted_credible_intervals_isotope_result_item.SetShowAsString(true);
    predicted_credible_intervals_isotope_result_item.SetShowTable(true);
    predicted_credible_intervals_isotope_result_item.SetType(result_type::rangeset_with_observed);
    predicted_credible_intervals_isotope_result_item.SetResult(predicted_credible_intervals_isotopes);
    predicted_credible_intervals_isotope_result_item.SetYAxisMode(yaxis_mode::normal);
    results.Append(predicted_credible_intervals_isotope_result_item);
    qDebug()<<"Done for sample "<< QString::fromStdString(sample);
    rtw_->SetProgress(1);
    return results;
}

CMBMatrix SourceSinkData::MCMC_Batch(map<string,string> arguments, CMCMC<SourceSinkData> *mcmc, ProgressWindow *rtw_, const string &workingfolder)
{
    InitializeParametersAndObservations(at(target_group_).begin()->first);
    CMBMatrix contributions(numberofsourcesamplesets_*4,at(target_group_).size());

    for (unsigned int i=0; i<numberofsourcesamplesets_; i++)
    {
        contributions.SetColumnLabel(i*4, samplesetsorder_[i] +"-"+"low");
        contributions.SetColumnLabel(i*4+1, samplesetsorder_[i] +"-"+"high");
        contributions.SetColumnLabel(i*4+2, samplesetsorder_[i] +"-"+"median");
        contributions.SetColumnLabel(i*4+3, samplesetsorder_[i] +"-"+"mean");
    }
    int counter = 0;
    for (map<string,Elemental_Profile>::iterator sample = at(target_group_).begin(); sample!=at(target_group_).end(); sample++)
    {
        qDebug()<<QString::fromStdString(sample->first)<<":"<<"Creating folder";
        QDir dir(QString::fromStdString(workingfolder)+"/"+QString::fromStdString(sample->first));
        if (!dir.exists())
            dir.mkpath(".");
        contributions.SetRowLabel(counter,sample->first);
        rtw_->SetLabel(QString::fromStdString(sample->first));
        qDebug()<<QString::fromStdString(sample->first)<<":"<<"Performing MCMC";
        Results results = MCMC(sample->first, arguments, mcmc, rtw_, workingfolder);
        qDebug()<<QString::fromStdString(sample->first)<<":"<<"Writing Result Items";
        for (map<string,ResultItem>::iterator result_item = results.begin(); result_item!=results.end(); result_item++ )
        {
            QString file_name = dir.absolutePath()+"/"+QString::fromStdString(result_item->first)+".txt";
            QFile file(file_name);
            file.open(QIODevice::WriteOnly | QIODevice::Text);
            result_item->second.Result()->writetofile(&file);
            file.close();
        }
        for (unsigned int i=0; i<numberofsourcesamplesets_; i++)
        {
            contributions[counter][4*i] = static_cast<RangeSet*>(results.at("3:Source Contribution Credible Intervals").Result())->at(samplesetsorder_[i]+"_contribution").Get(_range::low);
            contributions[counter][4*i+1] = static_cast<RangeSet*>(results.at("3:Source Contribution Credible Intervals").Result())->at(samplesetsorder_[i]+"_contribution").Get(_range::high);
            contributions[counter][4*i+2] = static_cast<RangeSet*>(results.at("3:Source Contribution Credible Intervals").Result())->at(samplesetsorder_[i]+"_contribution").Median();
            contributions[counter][4*i+3] = static_cast<RangeSet*>(results.at("3:Source Contribution Credible Intervals").Result())->at(samplesetsorder_[i]+"_contribution").Mean();
        }

        counter++;
        rtw_->SetProgress2(double(counter)/double(at(target_group_).size()));
        rtw_->ClearGraph(0);
        rtw_->ClearGraph(1);
        rtw_->ClearGraph(2);
    }
    rtw_->SetProgress2(1);
    return contributions;
}

bool SourceSinkData::ToolsUsed(const string &toolname)
{
    std::list<string>::iterator iter = std::find (tools_used.begin(), tools_used.end(), toolname);
    if (iter == tools_used.end())
        return false;
    else
        return true;
}

string SourceSinkData::FirstOMConstituent()
{
    for (map<string,element_information>::iterator element = element_information_.begin(); element!=element_information_.end(); element++)
    {
        if (element->second.Role == element_information::role::organic_carbon)
            return element->first;
    }
    return "";
}


string SourceSinkData::FirstSizeConstituent()
{
    for (map<string,element_information>::iterator element = element_information_.begin(); element!=element_information_.end(); element++)
    {
        if (element->second.Role == element_information::role::particle_size)
            return element->first;
    }
    return "";
}

CMatrix SourceSinkData::WithinGroupCovarianceMatrix()
{
    vector<string> elementNames = GetElementNames();
    CMatrix CovMatr(elementNames.size());
    int counter = 0;
    for (map<string,Elemental_Profile_Set>::iterator source_group = begin(); source_group!=end(); source_group++)
    {
        if (source_group->first != target_group_)
        {   CovMatr += (source_group->second.size()-1)*source_group->second.CalculateCovarianceMatrix();
            counter += source_group->second.size()-1;
        }
    }
    return CovMatr/double(counter);
}

CMatrix SourceSinkData::BetweenGroupCovarianceMatrix()
{
    vector<string> elementNames = GetElementNames();
    CMatrix out(elementNames.size());
    double count = 0;
    for (map<string,Elemental_Profile_Set>::iterator source_group = begin(); source_group!=end(); source_group++)
    {
        if (source_group->first != target_group_)
        {
            CMBVector deviation = MeanElementalContent() - MeanElementalContent(source_group->first);
            for (int i=0; i<elementNames.size(); i++)
                for (int j=0; j<elementNames.size(); j++)
                    out[i][j] += deviation[i]*deviation[j]*source_group->second.size();
            count += source_group->second.size();
        }
    }
    return out/count;
}

CMatrix SourceSinkData::TotalScatterMatrix()
{
    vector<string> elementNames = GetElementNames();
    CMatrix out(elementNames.size());
    double count = 0;
    CMBVector OverallMean = MeanElementalContent();
    for (map<string,Elemental_Profile_Set>::iterator source_group = begin(); source_group!=end(); source_group++)
    {
        if (source_group->first != target_group_)
        {
            for (map<string,Elemental_Profile>::iterator elem_prof = source_group->second.begin(); elem_prof!=source_group->second.end(); elem_prof++ )
            {
                for (int i=0; i<elementNames.size(); i++)
                    for (int j=0; j<elementNames.size(); j++)
                        out[i][j] += (OverallMean[i]-elem_prof->second.at(elementNames[i]))*(OverallMean[j]-elem_prof->second.at(elementNames[j]));
            }
            count += source_group->second.size();
        }
    }
    return out/count;
}

double SourceSinkData::WilksLambda()
{
    CMatrix_arma S_w = WithinGroupCovarianceMatrix();
    CMatrix_arma S_B = BetweenGroupCovarianceMatrix();
    CMatrix_arma S_T = S_w + S_B;
   
    double numerator = S_w.det();
    double denumerator = S_T.det();
    return fabs(numerator)/fabs(denumerator);
}

double SourceSinkData::DFA_P_Value()
{
    int element_count = GetElementNames().size();
    double wilkslambda = min(WilksLambda(),1.0);
    double ChiSquared = -(TotalNumberofSourceSamples() - 1 - (element_count+(numberofsourcesamplesets_-1.0)/2.0))*log(wilkslambda);
    double df;
    if (target_group_!="")
        df = element_count*(numberofsourcesamplesets_ - 2.0);
    else
        df = element_count*(numberofsourcesamplesets_ - 1.0);
    double p_value = gsl_cdf_chisq_Q (ChiSquared, df);
    return p_value;
}

CMBVectorSet SourceSinkData::DFA_Projected()
{
    CMBVector eigen_vector = DFA_eigvector();
    CMBVectorSet out;
    for (map<string,Elemental_Profile_Set>::iterator source_group = begin(); source_group!=end(); source_group++)
    {
        CMBVector weighted = source_group->second.CalculateDotProduct(eigen_vector);
        out.Append(source_group->first,weighted);
    }
    return out;
}


CMBVectorSet SourceSinkData::DFA_Projected(const string &source1, const string &source2)
{
    CMBVector eigen_vector = DFA_eigvector();
    CMBVectorSet out;
    for (map<string,Elemental_Profile_Set>::iterator source_group = begin(); source_group!=end(); source_group++)
    {
        CMBVector weighted = source_group->second.CalculateDotProduct(eigen_vector);
        out.Append(source_group->first,weighted);
    }
    return out;
}

CMBVectorSet SourceSinkData::DFA_Projected(const string &source1, SourceSinkData *original)
{
    CMBVector eigen_vector = DFA_eigvector();
    CMBVectorSet out;
    for (map<string,Elemental_Profile_Set>::iterator source_group = original->begin(); source_group!=original->end(); source_group++)
    {
        if (source_group->first != original->target_group_)
        {   CMBVector weighted = source_group->second.CalculateDotProduct(eigen_vector);
            out.Append(source_group->first,weighted);
        }
    }
    return out;
}

CMBVector SourceSinkData::DFA_eigvector()
{
    CMatrix_arma S_B = BetweenGroupCovarianceMatrix();
    CMatrix_arma S_w = WithinGroupCovarianceMatrix();
    CMatrix_arma InvS_w = inv(S_w);
    if (InvS_w.getnumrows()==0)
        return CMBVector();
    CMatrix_arma Product = inv(S_w)*S_B;

    arma::cx_vec eigval;
    arma::cx_mat eigvec;
    eig_gen(eigval, eigvec, Product);


    CVector_arma Eigvals = GetReal(eigval);
    CMatrix_arma Eigvecs = GetReal(eigvec);

    
    CVector_arma EigvalsImg = GetImg(eigval);
    CMatrix_arma EigvecsImg = GetImg(eigvec);
    //Eigvecs.writetofile("Eigvecs.txt");
    //Eigvals.writetofile("Eigvals.txt");
    //EigvalsImg.writetofile("EigvalsImg.txt");
    //EigvecsImg.writetofile("EigvecsImg.txt");
    

    CMBVector out = CVector_arma(Eigvecs.getcol(Eigvals.abs_max_elems()));
    
    vector<string> elementNames = GetElementNames();
    out.SetLabels(elementNames);
    return out;
}

CMBVector SourceSinkData::DFA_weight_vector(const string &source1, const string &source2)
{
    CMatrix_arma Sigma1 = at(source1).CalculateCovarianceMatrix();
    CMatrix_arma Sigma2 = at(source2).CalculateCovarianceMatrix();
    //Sigma1.writetofile("Sigma1.txt");
    //Sigma2.writetofile("Sigma2.txt");
    CMatrix_arma Sigma = Sigma1 + Sigma2;
    //Invert(Sigma).writetofile("Inv_Sigma.txt");
    CVector mu1_vec = at(source1).CalculateElementMeans();
    CVector mu2_vec = at(source2).CalculateElementMeans();
    CVector_arma mu1 = mu1_vec;
    CVector_arma mu2 = mu2_vec;
    CVector out_arma = (mu2-mu1)/(Sigma1+Sigma2);
    CMBVector out = out_arma;
    out = out/out.norm2();
    out.SetLabels(GetElementNames());
    return out;
}

CMBVector SourceSinkData::MeanElementalContent(const string &group_name)
{
    CMBVector out;
    if (count(group_name)==0)
        return out;
    vector<string> elementNames = GetElementNames();
    out = at(group_name).CalculateElementMeans();
    out.SetLabels(elementNames);
    return out;
}

CMBVector SourceSinkData::MeanElementalContent()
{
    vector<string> elementNames = GetElementNames();
    CMBVector out(elementNames.size());
    double count = 0;
    for (map<string,Elemental_Profile_Set>::iterator source_group = begin(); source_group!=end(); source_group++)
    {
        if (source_group->first != target_group_)
        {
            out += double(source_group->second.size())*MeanElementalContent(source_group->first);
            count += double(source_group->second.size());
        }
    }
    out.SetLabels(element_order_);
    return out/count;
}

vector<CMBVector> SourceSinkData::StepwiseDiscriminantFunctionAnalysis(const string &source1, const string &source2)
{
    vector<CMBVector> out(3);
    vector<string> elemnames = GetElementNames();
    vector<string> selected_labels;
    for (unsigned int i=0; i<elemnames.size(); i++)
    {
        if (rtw_)
            rtw_->SetProgress(double(i + 1) / double(elemnames.size()));
        double min_P = 100;
        string highestimproved;
        double wilkslambda;
        double F_test_p_value;
        for (unsigned int j=0; j<elemnames.size(); j++)
        {
            vector<string> selected_labels_temp = selected_labels;
            if (lookup(selected_labels,elemnames[j])==-1)
            {
                selected_labels_temp.push_back(elemnames[j]);
                SourceSinkData tobeanalysed = ExtractSpecificElements(selected_labels_temp);
                DFA_result thisDFAresults = tobeanalysed.DiscriminantFunctionAnalysis(source1,source2);
                if (thisDFAresults.eigen_vectors.size()==0)
                    return out;
                if (thisDFAresults.p_values[0]<min_P)
                {
                    highestimproved = elemnames[j];
                    min_P = thisDFAresults.p_values[0];
                    wilkslambda = thisDFAresults.wilkslambda[0];
                    F_test_p_value = thisDFAresults.F_test_P_value[0];
                }
            }
        }
        out[0].append(highestimproved,min_P);
        out[1].append(highestimproved,wilkslambda);
        out[2].append(highestimproved,F_test_p_value);
        selected_labels.push_back(highestimproved);

    }
    return out;
}

vector<CMBVector> SourceSinkData::StepwiseDiscriminantFunctionAnalysis()
{
    vector<CMBVector> out(3);
    vector<string> elemnames = GetElementNames();
    vector<string> selected_labels;
    for (unsigned int i=0; i<elemnames.size(); i++)
    {
        if (rtw_)
            rtw_->SetProgress(double(i + 1) / double(elemnames.size()));
        double min_P = 100;
        string highestimproved;
        double wilkslambda;
        double F_test_p_value;
        for (unsigned int j=0; j<elemnames.size(); j++)
        {
            vector<string> selected_labels_temp = selected_labels;
            if (lookup(selected_labels,elemnames[j])==-1)
            {
                selected_labels_temp.push_back(elemnames[j]);
                SourceSinkData tobeanalysed = ExtractSpecificElements(selected_labels_temp);
                double p_value = tobeanalysed.DFA_P_Value();

                if (p_value<min_P)
                {
                    highestimproved = elemnames[j];
                    min_P = p_value;
                    wilkslambda = tobeanalysed.WilksLambda();
                    CMBVectorSet projected = tobeanalysed.DFA_Projected();
                    F_test_p_value = projected.FTest_p_value();
                }
            }
        }
        out[0].append(highestimproved,min_P);
        out[1].append(highestimproved,wilkslambda);
        out[2].append(highestimproved,F_test_p_value);
        selected_labels.push_back(highestimproved);

    }
    return out;
}

vector<CMBVector> SourceSinkData::StepwiseDiscriminantFunctionAnalysis(const string &source1)
{
    vector<CMBVector> out(3);
    vector<string> elemnames = GetElementNames();
    vector<string> selected_labels;
    for (unsigned int i=0; i<elemnames.size(); i++)
    {
        if (rtw_)
            rtw_->SetProgress(double(i + 1) / double(elemnames.size()));
        double min_P = 100;
        string highestimproved;
        double wilkslambda;
        double F_test_p_value;
        for (unsigned int j=0; j<elemnames.size(); j++)
        {
            vector<string> selected_labels_temp = selected_labels;
            if (lookup(selected_labels,elemnames[j])==-1)
            {
                selected_labels_temp.push_back(elemnames[j]);
                SourceSinkData tobeanalysed = ExtractSpecificElements(selected_labels_temp);
                DFA_result thisDFAresults = tobeanalysed.DiscriminantFunctionAnalysis(source1);
                if (thisDFAresults.p_values[0]<min_P)
                {
                    highestimproved = elemnames[j];
                    min_P = thisDFAresults.p_values[0];
                    wilkslambda = thisDFAresults.wilkslambda[0];
                    F_test_p_value = thisDFAresults.F_test_P_value[0];
                }
            }
        }
        out[0].append(highestimproved,min_P);
        out[1].append(highestimproved,wilkslambda);
        out[2].append(highestimproved,F_test_p_value);
        selected_labels.push_back(highestimproved);

    }
    return out;
}
