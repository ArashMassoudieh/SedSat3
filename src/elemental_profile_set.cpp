#include "elemental_profile_set.h"
#include <iostream>
#include <gsl/gsl_statistics_double.h>
#include <QFile>

// ========== Construction and Assignment ==========

Elemental_Profile_Set::Elemental_Profile_Set()
    : map<string, Elemental_Profile>(),
    Interface(),
    element_distributions_(),
    mlr_vs_om_size_(),
    contribution_(0.0),
    contribution_softmax_(0.0),
    outlierdone_(false)
{
}

Elemental_Profile_Set::Elemental_Profile_Set(const Elemental_Profile_Set& other)
    : map<string, Elemental_Profile>(other),
    Interface(other),
    element_distributions_(other.element_distributions_),
    mlr_vs_om_size_(other.mlr_vs_om_size_),
    contribution_(other.contribution_),
    contribution_softmax_(other.contribution_softmax_),
    outlierdone_(other.outlierdone_)
{
}

Elemental_Profile_Set& Elemental_Profile_Set::operator=(const Elemental_Profile_Set& other)
{
    // Check for self-assignment
    if (this == &other) {
        return *this;
    }

    // Copy base classes
    map<string, Elemental_Profile>::operator=(other);
    Interface::operator=(other);

    // Copy member variables
    element_distributions_ = other.element_distributions_;
    mlr_vs_om_size_ = other.mlr_vs_om_size_;
    contribution_ = other.contribution_;
    contribution_softmax_ = other.contribution_softmax_;
    outlierdone_ = other.outlierdone_;

    return *this;
}
// ========== Data Extraction and Filtering ==========

Elemental_Profile_Set Elemental_Profile_Set::CreateCorrectedSet(
    bool exclude_samples,
    bool exclude_elements,
    bool omnsizecorrect,
    const vector<double>& om_size,
    const map<string, element_information>* elementinfo) const
{
    Elemental_Profile_Set out;

    for (const auto& [sample_name, profile] : *this)
    {
        // Skip excluded samples if requested
        if (exclude_samples && !profile.IsIncludedInAnalysis()) {
            continue;
        }

        // Apply corrections based on whether MLR models exist
        if (!mlr_vs_om_size_.empty()) {
            out.AppendProfile(
                sample_name,
                profile.CreateCorrectedProfile(exclude_elements, omnsizecorrect, om_size, &mlr_vs_om_size_, elementinfo)
            );
        }
        else {
            out.AppendProfile(
                sample_name,
                profile.CreateCorrectedProfile(exclude_elements, false, om_size, nullptr, elementinfo)
            );
        }
    }

    out.SetRegressionModels(&mlr_vs_om_size_);
    return out;
}

Elemental_Profile_Set Elemental_Profile_Set::ExtractChemicalElements(
    const map<string, element_information>* elementinfo,
    bool isotopes) const
{
    Elemental_Profile_Set out;

    for (const auto& [sample_name, profile] : *this)
    {
        out.AppendProfile(sample_name, profile.ExtractChemicalElements(elementinfo, isotopes));
    }

    return out;
}

Elemental_Profile_Set Elemental_Profile_Set::ExtractElements(
    const vector<string>& element_list) const
{
    Elemental_Profile_Set out;

    for (const auto& [sample_name, profile] : *this)
    {
        out.AppendProfile(sample_name, profile.ExtractElements(element_list));
    }

    return out;
}

Elemental_Profile_Set Elemental_Profile_Set::CopyIncludedInAnalysis(
    bool applyomsizecorrection,
    const vector<double>& om_size,
    map<string, element_information>* elementinfo)
{
    Elemental_Profile_Set out;

    if (applyomsizecorrection)
    {
        out = ApplyOrganicMatterAndSizeCorrections(om_size, elementinfo);
    }
    else
    {
        for (auto& [sample_name, profile] : *this)
        {
            // Include only samples marked for analysis with non-empty names
            if (profile.IsIncludedInAnalysis() && !sample_name.empty())
            {
                out.AppendProfile(sample_name, profile, elementinfo);
            }
        }
    }

    out.SetRegressionModels(&mlr_vs_om_size_);
    return out;
}

Elemental_Profile_Set Elemental_Profile_Set::EliminateSamples(
    vector<string> samples_to_eliminate,
    map<string, element_information>* elementinfo) const
{
    Elemental_Profile_Set out;

    for (const auto& [sample_name, profile] : *this)
    {
        // Include sample if it's marked for analysis AND not in elimination list
        if (profile.IsIncludedInAnalysis() && lookup(samples_to_eliminate, sample_name) == -1)
        {
            out.AppendProfile(sample_name, profile, elementinfo);
        }
    }

    out.SetRegressionModels(&mlr_vs_om_size_);
    return out;
}

// ========== Data Management ==========

void Elemental_Profile_Set::UpdateElementDistributions()
{
    element_distributions_.clear();

    // Iterate through each sample profile
    for (auto& [sample_name, profile] : *this)
    {
        // Iterate through each element in the profile
        for (const auto& [element_name, concentration] : profile)
        {
            element_distributions_[element_name].AppendValue(concentration);
        }
    }
}

// ========== Data Management ==========

Elemental_Profile* Elemental_Profile_Set::AppendProfile(
    const string& name,
    const Elemental_Profile& profile,
    map<string, element_information>* elementinfo)
{
    // Check if profile with this name already exists
    if (count(name) > 0)
    {
        std::cerr << "Profile '" + name + "' already exists!" << std::endl;
        return nullptr;
    }

    // Add the profile (filtered by elementinfo if provided)
    operator[](name) = profile.CreateAnalysisProfile(elementinfo);

    // Update element distributions
    for (const auto& [element_name, concentration] : profile)
    {
        // No filtering - add all elements
        if (elementinfo == nullptr)
        {
            element_distributions_[element_name].AppendValue(concentration);
        }
        // With filtering - only add elements that meet criteria
        else if (elementinfo->count(element_name) != 0)
        {
            const auto& elem_info = elementinfo->at(element_name);

            // Include if: marked for analysis AND not excluded roles
            if (elem_info.include_in_analysis &&
                elem_info.Role != element_information::role::do_not_include &&
                elem_info.Role != element_information::role::particle_size &&
                elem_info.Role != element_information::role::organic_carbon)
            {
                element_distributions_[element_name].AppendValue(concentration);
            }
        }
    }

    return &operator[](name);
}

void Elemental_Profile_Set::AppendProfiles(
    const Elemental_Profile_Set& profiles,
    map<string, element_information>* elementinfo)
{
    // Add each profile from the source set
    for (const auto& [sample_name, profile] : profiles)
    {
        AppendProfile(sample_name, profile, elementinfo);
    }

    // Rebuild distributions after adding all profiles
    UpdateElementDistributions();
}

vector<string> Elemental_Profile_Set::GetSampleNames() const
{
    vector<string> sample_names;
    sample_names.reserve(size());  // Pre-allocate for efficiency

    for (const auto& [sample_name, profile] : *this)
    {
        sample_names.push_back(sample_name);
    }

    return sample_names;
}

// ========== Serialization ==========

string Elemental_Profile_Set::ToString()
{
    if (empty()) {
        return string();
    }

    string out;
    vector<string> element_names = GetElementNames();

    // Header row: "Element name" followed by sample names
    out += "Element name\t";
    for (const auto& [sample_name, profile] : *this)
    {
        out += sample_name + "\t";
    }
    out += "\n";

    // Data rows: element name followed by concentrations for each sample
    for (const string& element : element_names)
    {
        out += element;
        for (const auto& [sample_name, profile] : *this)
        {
            out += "\t" + aquiutils::numbertostring(profile.at(element));
        }
        out += "\n";
    }

    return out;
}

bool Elemental_Profile_Set::writetofile(QFile* file)
{
    if (!file) {
        return false;
    }

    file->write(QString::fromStdString(ToString()).toUtf8());
    return true;  // Fixed: was returning 0 (false)
}

QJsonObject Elemental_Profile_Set::toJsonObject()
{
    QJsonObject json_object;

    if (empty()) {
        return json_object;
    }

    for (auto& [sample_name, profile] : *this)
    {
        // Only include samples with non-empty names
        if (!sample_name.empty())
        {
            json_object[QString::fromStdString(sample_name)] = profile.toJsonObject();
        }
    }

    return json_object;
}

bool Elemental_Profile_Set::ReadFromJsonObject(const QJsonObject& jsonobject)
{
    clear();

    for (const QString& key : jsonobject.keys())
    {
        if (key.isEmpty()) {
            continue;
        }

        Elemental_Profile elemental_profile;
        if (elemental_profile.ReadFromJsonObject(jsonobject[key].toObject()))
        {
            AppendProfile(key.toStdString(), elemental_profile);
        }
    }

    return true;
}
bool Elemental_Profile_Set::Read(const QStringList &strlist)
{


    return true;
}

// ========== Data Access ==========

bool Elemental_Profile_Set::ContainsElement(const string& element_name) const
{
    if (empty()) {
        return false;
    }

    // Element must exist in all profiles
    for (const auto& [sample_name, profile] : *this)
    {
        if (!profile.Contains(element_name)) {
            return false;
        }
    }

    return true;
}

vector<string> Elemental_Profile_Set::GetElementNames() const
{
    if (empty()) {
        return vector<string>();
    }

    // Get element names from the first profile
    vector<string> element_names;
    const Elemental_Profile& first_profile = begin()->second;
    element_names.reserve(first_profile.size());

    for (const auto& [element_name, concentration] : first_profile)
    {
        element_names.push_back(element_name);
    }

    return element_names;
}

Elemental_Profile* Elemental_Profile_Set::GetProfile(const string& name)
{
    if (count(name) == 0)
    {
        std::cerr << "Sample '" + name + "' does not exist!" << std::endl;
        return nullptr;
    }

    return &operator[](name);
}

Elemental_Profile* Elemental_Profile_Set::GetProfile(unsigned int index)
{
    if (index >= size()) {
        return nullptr;
    }

    // Advance iterator to the requested index
    auto it = begin();
    std::advance(it, index);

    return &it->second;
}

Elemental_Profile Elemental_Profile_Set::GetProfile(const string& name) const
{
    if (count(name) == 0)
    {
        std::cerr << "Sample '" + name + "' does not exist!" << std::endl;
        return Elemental_Profile();
    }

    return at(name);
}

Elemental_Profile Elemental_Profile_Set::GetProfile(unsigned int index) const
{
    if (index >= size()) {
        return Elemental_Profile();
    }

    // Advance iterator to the requested index
    auto it = begin();
    std::advance(it, index);

    return it->second;
}

// ========== Data Access (continued) ==========

vector<double> Elemental_Profile_Set::GetAllConcentrationsFor(const string& element_name)
{
    vector<double> concentrations;

    // Check if element exists in first profile
    if (empty() || begin()->second.count(element_name) == 0) {
        return concentrations;
    }

    concentrations.reserve(size());

    for (const auto& [sample_name, profile] : *this)
    {
        concentrations.push_back(profile.at(element_name));
    }

    return concentrations;
}

vector<double> Elemental_Profile_Set::GetConcentrationsForSample(const string& sample_name) const
{
    // Check if sample exists
    if (count(sample_name) == 0) {
        return vector<double>();
    }

    // Use at() to get const reference (no copy)
    return at(sample_name).GetAllValues();
}

double Elemental_Profile_Set::GetMaximum() const
{
    if (empty()) {
        return 0.0;
    }

    double maximum = -std::numeric_limits<double>::max();

    for (const auto& [sample_name, profile] : *this)
    {
        double profile_max = profile.GetMaximum();
        if (profile_max > maximum) {
            maximum = profile_max;
        }
    }

    return maximum;
}

double Elemental_Profile_Set::GetMinimum() const
{
    if (empty()) {
        return 0.0;
    }

    double minimum = std::numeric_limits<double>::max();

    for (const auto& [sample_name, profile] : *this)
    {
        double profile_min = profile.GetMinimum();
        if (profile_min < minimum) {
            minimum = profile_min;
        }
    }

    return minimum;
}
// ========== Regression Models (OM/Size Corrections) ==========

MultipleLinearRegressionSet Elemental_Profile_Set::BuildRegressionModels(
    const string& om,
    const string& d,
    regression_form form,
    const double& p_value_threshold)
{
    MultipleLinearRegressionSet models;
    vector<string> element_names = GetElementNames();

    for (const string& element : element_names)
    {
        models[element] = BuildRegressionModel(element, om, d, form, p_value_threshold);
        models[element].SetDependentVariableName(element);
    }

    return models;
}

MultipleLinearRegression Elemental_Profile_Set::BuildRegressionModel(
    const string& element,
    const string& om,
    const string& d,
    regression_form form,
    const double& p_value_threshold)
{
    MultipleLinearRegression model;

    // Get dependent variable (element concentrations)
    vector<double> dependent = GetAllConcentrationsFor(element);

    // Build independent variables (OM and/or particle size)
    vector<vector<double>> independents;
    vector<string> independent_var_names;

    if (!om.empty()) {
        independents.push_back(GetAllConcentrationsFor(om));
        independent_var_names.push_back(om);
    }

    if (!d.empty()) {
        independents.push_back(GetAllConcentrationsFor(d));
        independent_var_names.push_back(d);
    }

    // Configure and run regression
    model.SetPValueThreshold(p_value_threshold);
    model.SetEquation(form);
    model.SetDependentVariableName(element);
    model.Regress(independents, dependent, independent_var_names);

    return model;
}

void Elemental_Profile_Set::SetRegressionModels(
    const string& om,
    const string& d,
    regression_form form,
    const double& p_value_threshold)
{
    mlr_vs_om_size_ = BuildRegressionModels(om, d, form, p_value_threshold);
}

void Elemental_Profile_Set::SetRegressionModels(const MultipleLinearRegressionSet* mlrset)
{
    if (mlrset) {
        mlr_vs_om_size_ = *mlrset;
    }
}

ResultItem Elemental_Profile_Set::GetRegressionsAsResult()
{
    ResultItem result;
    result.SetResult(new MultipleLinearRegressionSet(mlr_vs_om_size_));
    result.SetType(result_type::mlrset);
    return result;
}

MultipleLinearRegressionSet* Elemental_Profile_Set::GetRegressionModels()
{
    return &mlr_vs_om_size_;
}

// ========== Statistical Analysis ==========

CMBMatrix Elemental_Profile_Set::CalculateCovarianceMatrix()
{
    vector<string> element_names = GetElementNames();
    CMBMatrix covariance_matrix(element_names.size());

    // Convert to GSL matrix for statistics calculations
    gsl_matrix* data_matrix = CopyToGSLMatrix();

    // Set matrix labels
    for (size_t i = 0; i < element_names.size(); i++)
    {
        covariance_matrix.SetColumnLabel(i, element_names[i]);
        covariance_matrix.SetRowLabel(i, element_names[i]);
    }

    // Calculate covariance for each element pair
    for (size_t i = 0; i < data_matrix->size2; i++)
    {
        for (size_t j = i; j < data_matrix->size2; j++)
        {
            gsl_vector_view col_i = gsl_matrix_column(data_matrix, i);
            gsl_vector_view col_j = gsl_matrix_column(data_matrix, j);

            // GSL uses n-1 denominator, but we want n denominator for consistency
            double cov = gsl_stats_covariance(
                col_i.vector.data, col_i.vector.stride,
                col_j.vector.data, col_j.vector.stride,
                col_i.vector.size
            );

            // Adjust from n-1 to n denominator
            double adjusted_cov = cov * (col_i.vector.size - 1) / col_i.vector.size;

            // Matrix is symmetric
            covariance_matrix[i][j] = adjusted_cov;
            covariance_matrix[j][i] = adjusted_cov;
        }
    }

    gsl_matrix_free(data_matrix);
    return covariance_matrix;
}

CMBMatrix Elemental_Profile_Set::CalculateCorrelationMatrix()
{
    vector<string> element_names = GetElementNames();
    CMBMatrix correlation_matrix(element_names.size());

    // Convert to GSL matrix for statistics calculations
    gsl_matrix* data_matrix = CopyToGSLMatrix();

    // Set matrix labels
    for (size_t i = 0; i < element_names.size(); i++)
    {
        correlation_matrix.SetColumnLabel(i, element_names[i]);
        correlation_matrix.SetRowLabel(i, element_names[i]);
    }

    // Calculate correlation for each element pair
    for (size_t i = 0; i < data_matrix->size2; i++)
    {
        for (size_t j = i; j < data_matrix->size2; j++)
        {
            gsl_vector_view col_i = gsl_matrix_column(data_matrix, i);
            gsl_vector_view col_j = gsl_matrix_column(data_matrix, j);

            double corr = gsl_stats_correlation(
                col_i.vector.data, col_i.vector.stride,
                col_j.vector.data, col_j.vector.stride,
                col_i.vector.size
            );

            // Matrix is symmetric
            correlation_matrix[i][j] = corr;
            correlation_matrix[j][i] = corr;
        }
    }

    gsl_matrix_free(data_matrix);
    return correlation_matrix;
}

gsl_matrix* Elemental_Profile_Set::CopyToGSLMatrix() const
{
    vector<string> element_names = GetElementNames();

    // Allocate matrix: rows = samples, columns = elements
    gsl_matrix* matrix = gsl_matrix_alloc(size(), element_names.size());

    // Fill matrix with concentration data
    size_t row = 0;
    for (const auto& [sample_name, profile] : *this)
    {
        size_t col = 0;
        for (const auto& [element_name, concentration] : profile)
        {
            gsl_matrix_set(matrix, row, col, concentration);
            col++;
        }
        row++;
    }

    return matrix;
}
// ========== Statistical Analysis (continued) ==========

CMBVector Elemental_Profile_Set::CalculateKolmogorovSmirnovStatistics(
    distribution_type dist_type)
{
    vector<string> element_names = GetElementNames();
    CMBVector ks_statistics(element_names.size());
    ks_statistics.SetLabels(element_names);

    for (size_t i = 0; i < element_names.size(); i++)
    {
        ks_statistics[i] = GetElementDistribution(element_names[i])->
            CalculateKolmogorovSmirnovStatistic(dist_type);
    }

    return ks_statistics;
}

CVector Elemental_Profile_Set::CalculateElementMeans(
    const vector<string>& element_order) const
{
    // Use provided element order, or get all elements if not specified
    vector<string> element_names = element_order.empty() ?
        GetElementNames() : element_order;

    CVector means(element_names.size());

    for (size_t i = 0; i < element_names.size(); i++)
    {
        means[i] = element_distributions_.at(element_names[i]).CalculateMean();
    }

    return means;
}

// ========== Serialization (continued) ==========

QTableWidget* Elemental_Profile_Set::ToTable()
{
    QTableWidget* table = new QTableWidget();
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    vector<string> element_names = GetElementNames();

    // Set table dimensions: rows = elements, columns = samples
    table->setRowCount(element_names.size());
    table->setColumnCount(size());

    // Build row headers (element names)
    QStringList row_headers;
    for (const string& element : element_names)
    {
        row_headers << QString::fromStdString(element);
    }
    table->setVerticalHeaderLabels(row_headers);

    // Fill table with concentration data
    QStringList column_headers;
    int col = 0;

    for (const auto& [sample_name, profile] : *this)
    {
        // Add column header (sample name)
        column_headers << QString::fromStdString(sample_name);

        // Fill column with element concentrations
        for (size_t row = 0; row < element_names.size(); row++)
        {
            double concentration = profile.GetValue(element_names[row]);
            table->setItem(row, col, new QTableWidgetItem(QString::number(concentration)));

            // Highlight values outside limits if enabled
            if (highlightoutsideoflimit)
            {
                if (concentration > highlimit || concentration < lowlimit)
                {
                    table->item(row, col)->setForeground(QColor(Qt::red));
                }
            }
        }

        col++;
    }

    table->setHorizontalHeaderLabels(column_headers);

    return table;
}
// ========== Regression Models (continued) ==========

Elemental_Profile_Set Elemental_Profile_Set::ApplyOrganicMatterAndSizeCorrections(
    const vector<double>& om_size,
    map<string, element_information>* elementinfo)
{
    Elemental_Profile_Set corrected_set;

    for (auto& [sample_name, profile] : *this)
    {
        // Only include samples marked for analysis with non-empty names
        if (profile.IsIncludedInAnalysis() && !sample_name.empty())
        {
            Elemental_Profile corrected_profile =
                profile.ApplyOrganicMatterAndSizeCorrections(om_size, &mlr_vs_om_size_, elementinfo);

            corrected_set.AppendProfile(sample_name, corrected_profile, elementinfo);
        }
    }

    return corrected_set;
}
// ========== Box-Cox Transformations ==========

CMBVector Elemental_Profile_Set::CalculateBoxCoxParameters()
{
    CMBVector lambda_values(element_distributions_.size());

    size_t i = 0;
    for (auto& [element_name, distribution] : element_distributions_)
    {
        lambda_values[i] = distribution.FindOptimalBoxCoxParameter(-5.0, 5.0, 10);
        lambda_values.SetLabel(i, element_name);
        i++;
    }

    return lambda_values;
}
// ========== Outlier Detection ==========

CMBMatrix Elemental_Profile_Set::DetectOutliers(
    const double& lowerlimit,
    const double& upperlimit)
{
    // Calculate optimal Box-Cox parameters for each element
    CMBVector lambdas = CalculateBoxCoxParameters();

    // Initialize output matrix: rows = samples, columns = elements
    CMBMatrix outlier_magnitude(begin()->second.size(), size());

    // Calculate means and standard deviations of Box-Cox transformed distributions
    vector<double> means;
    vector<double> stds;
    means.reserve(element_distributions_.size());
    stds.reserve(element_distributions_.size());

    size_t elem_idx = 0;
    for (auto& [element_name, distribution] : element_distributions_)
    {
        ConcentrationSet transformed = distribution.ApplyBoxCoxTransform(lambdas[elem_idx], false);
        means.push_back(transformed.CalculateMean());
        stds.push_back(transformed.CalculateStdDev());
        elem_idx++;
    }

    // Calculate standardized outlier magnitude for each sample
    size_t sample_idx = 0;
    for (auto& [sample_name, profile] : *this)
    {
        profile.ClearNotes();
        outlier_magnitude.SetRowLabel(sample_idx, sample_name);

        size_t element_idx = 0;
        for (const auto& [element_name, concentration] : profile)
        {
            outlier_magnitude.SetColumnLabel(element_idx, element_name);

            // Apply Box-Cox transformation
            double lambda = lambdas[element_idx];
            double transformed_value = (pow(concentration, lambda) - 1.0) / lambda;

            // Calculate standardized score
            double z_score = (transformed_value - means[element_idx]) / stds[element_idx];
            outlier_magnitude[sample_idx][element_idx] = z_score;

            // Flag outliers if thresholds are set
            if (upperlimit != lowerlimit)
            {
                if (z_score > upperlimit || z_score < lowerlimit)
                {
                    profile.AppendtoNotes(element_name + " was detected as outlier");
                }
            }

            element_idx++;
        }

        sample_idx++;
    }

    outlierdone_ = true;
    return outlier_magnitude;
}

// ========== Box-Cox Transformations (continued) ==========

Elemental_Profile_Set Elemental_Profile_Set::ApplyBoxCoxTransform(CMBVector* lambda_vals)
{
    // Use provided lambdas or calculate optimal ones
    CMBVector lambdas = (lambda_vals == nullptr) ?
        CalculateBoxCoxParameters() : *lambda_vals;

    // Create copy of this set to transform
    Elemental_Profile_Set transformed(*this);

    // Apply Box-Cox transformation to each element across all samples
    size_t elem_idx = 0;
    for (auto& [element_name, distribution] : element_distributions_)
    {
        // Transform all values for this element
        ConcentrationSet transformed_values =
            distribution.ApplyBoxCoxTransform(lambdas[elem_idx], false);

        // Update each profile with transformed value
        size_t sample_idx = 0;
        for (auto& [sample_name, profile] : transformed)
        {
            profile[element_name] = transformed_values[sample_idx];
            sample_idx++;
        }

        elem_idx++;
    }

    return transformed;
}

CMBMatrix Elemental_Profile_Set::ToMatrix() const
{
    vector<string> element_names = GetElementNames();

    // Initialize matrix: rows = samples, columns = elements
    CMBMatrix matrix(size(), element_names.size());

    // Fill matrix with concentration data
    size_t row = 0;
    for (const auto& [sample_name, profile] : *this)
    {
        matrix.SetRowLabel(row, sample_name);

        size_t col = 0;
        for (const auto& [element_name, concentration] : profile)
        {
            matrix[row][col] = concentration;
            matrix.SetColumnLabel(col, element_name);
            col++;
        }

        row++;
    }

    return matrix;
}

// ========== Outlier Detection (continued) ==========

vector<string> Elemental_Profile_Set::CheckForNegativeValues(
    const vector<string>& element_names) const
{
    vector<string> elements_with_negative_values;

    for (const string& element : element_names)
    {
        if (GetElementDistribution(element).GetMinimum() <= 0)
        {
            elements_with_negative_values.push_back(element);
        }
    }

    return elements_with_negative_values;
}

// ========== Utility Functions (continued) ==========

Elemental_Profile Elemental_Profile_Set::SelectTopElementsAggregate(int n) const
{
    Elemental_Profile aggregated;

    for (const auto& [sample_name, profile] : *this)
    {
        // Get top n elements from this profile sorted by concentration
        CMBVector sorted = profile.SortByConcentration();

        for (int i = 0; i < n && i < sorted.num; i++)
        {
            string element_name = sorted.Label(i);
            double concentration = sorted[i];

            // Add element if not present, or keep minimum concentration if already present
            if (aggregated.count(element_name) == 0)
            {
                aggregated.AppendElement(element_name, concentration);
            }
            else
            {
                aggregated[element_name] = std::min(concentration, aggregated[element_name]);
            }
        }
    }

    return aggregated;
}
// ========== Utility Functions (continued) ==========

CMBVector Elemental_Profile_Set::CalculateDotProduct(const CVector& v) const
{
    CMBVector dot_products(size());

    size_t i = 0;
    for (const auto& [sample_name, profile] : *this)
    {
        dot_products[i] = profile.CalculateDotProduct(v);
        dot_products.SetLabel(i, sample_name);
        i++;
    }

    return dot_products;
}

// Add this to elemental_profile_set.cpp

ConcentrationSet* Elemental_Profile_Set::GetElementDistribution(const string& element_name)
{
    if (element_distributions_.count(element_name) > 0) {
        return &element_distributions_[element_name];
    }
    return nullptr;
}

ConcentrationSet Elemental_Profile_Set::GetElementDistribution(const string& element_name) const
{
    return element_distributions_.at(element_name);
}

bool Elemental_Profile_Set::SetContributionSoftmax(const double& x)
{
    contribution_softmax_ = x;
    return true;
}

Distribution* Elemental_Profile_Set::GetEstimatedDistribution(const string& element_name)
{
    if (element_distributions_.count(element_name) > 0)
        return element_distributions_[element_name].GetEstimatedDistribution();
    else
        return nullptr;
}

Distribution* Elemental_Profile_Set::GetFittedDistribution(const string& element_name)
{
    if (element_distributions_.count(element_name) > 0)
        return element_distributions_[element_name].GetFittedDistribution();
    else
        return nullptr;
}

bool Elemental_Profile_Set::SetContribution(const double& x)
{
    contribution_ = x;
    return true;
}
