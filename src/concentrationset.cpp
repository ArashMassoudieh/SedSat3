// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#include "concentrationset.h"
#include <limits>
#include "math.h"
#include "Utilities.h"
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_randist.h>

// ========== Construction and Assignment ==========

ConcentrationSet::ConcentrationSet()
    : vector<double>(),
    fitted_distribution_(),
    estimated_distribution_()
{
}

ConcentrationSet::ConcentrationSet(const ConcentrationSet& other)
    : vector<double>(other),
    fitted_distribution_(other.fitted_distribution_),
    estimated_distribution_(other.estimated_distribution_)
{
}

ConcentrationSet::ConcentrationSet(int n)
    : vector<double>(n),
    fitted_distribution_(),
    estimated_distribution_()
{
}

ConcentrationSet& ConcentrationSet::operator=(const ConcentrationSet& other)
{
    if (this == &other) {
        return *this;
    }

    vector<double>::operator=(other);
    fitted_distribution_ = other.fitted_distribution_;
    estimated_distribution_ = other.estimated_distribution_;

    return *this;
}

// ========== Data Management ==========

void ConcentrationSet::AppendValue(double value)
{
    push_back(value);
}

void ConcentrationSet::AppendSet(const ConcentrationSet& other)
{
    insert(end(), other.begin(), other.end());
}

void ConcentrationSet::AppendSet(ConcentrationSet* other)
{
    if (other != nullptr) {
        insert(end(), other->begin(), other->end());
    }
}

// ========== Basic Statistics ==========

double ConcentrationSet::CalculateMean() const
{
    if (empty()) {
        return 0.0;
    }

    double sum = 0.0;
    for (const double value : *this) {
        sum += value;
    }

    return sum / static_cast<double>(size());
}

double ConcentrationSet::CalculateStdDev(double mean_value) const
{
    if (size() <= 1) {
        return 0.0;
    }

    if (mean_value == -999) {
        mean_value = CalculateMean();
    }

    double sum = 0.0;
    for (const double value : *this) {
        sum += pow(value - mean_value, 2);
    }

    return sqrt(sum / static_cast<double>(size() - 1));
}

double ConcentrationSet::CalculateStdDevLog(double mean_value) const
{
    if (size() <= 1) {
        return 0.0;
    }

    if (mean_value == -999) {
        mean_value = CalculateMeanLog();
    }

    double sum = 0.0;
    for (const double value : *this) {
        sum += pow(log(value) - mean_value, 2);
    }

    return sqrt(sum / static_cast<double>(size() - 1));
}

double ConcentrationSet::CalculateSSE(double mean_value) const
{
    if (mean_value == -999) {
        mean_value = CalculateMean();
    }

    double sum = 0.0;
    for (const double value : *this) {
        sum += pow(value - mean_value, 2);
    }

    return sum;
}

double ConcentrationSet::CalculateSSELog(double mean_value) const
{
    if (mean_value == -999) {
        mean_value = CalculateMeanLog();
    }

    double sum = 0.0;
    for (const double value : *this) {
        sum += pow(log(value) - mean_value, 2);
    }

    return sum;
}

double ConcentrationSet::CalculateMeanLog() const
{
    if (empty()) {
        return 0.0;
    }

    double sum = 0.0;
    for (const double value : *this) {
        sum += log(value);
    }

    return sum / static_cast<double>(size());
}

double ConcentrationSet::CalculateNorm(double power) const
{
    double sum = 0.0;
    for (const double value : *this) {
        sum += pow(value, power);
    }

    return sum;
}

double ConcentrationSet::CalculateNormLog(double power) const
{
    double sum = 0.0;
    for (const double value : *this) {
        sum += pow(log(value), power);
    }

    return sum;
}

ConcentrationSet ConcentrationSet::CreateLogTransformed() const
{
    ConcentrationSet transformed(size());

    for (size_t i = 0; i < size(); i++) {
        transformed[i] = log(at(i));
    }

    return transformed;
}

double ConcentrationSet::GetMinimum() const
{
    return aquiutils::Min(*this);
}

double ConcentrationSet::GetMaximum() const
{
    return aquiutils::Max(*this);
}

// ========== Distribution Fitting ==========

vector<double> ConcentrationSet::EstimateDistributionParameters(
    distribution_type dist_type)
{
    vector<double> parameters;

    // Use fitted distribution type if none specified
    if (dist_type == distribution_type::none) {
        dist_type = fitted_distribution_.distribution;
    }
    else {
        fitted_distribution_.SetType(dist_type);
        estimated_distribution_.SetType(dist_type);
    }

    // Store data statistics
    fitted_distribution_.SetDataMean(CalculateMean());
    fitted_distribution_.SetDataSTDev(CalculateStdDev());

    // Calculate distribution parameters based on type
    if (dist_type == distribution_type::normal) {
        parameters.push_back(CalculateMean());
        parameters.push_back(CalculateStdDev());
    }
    else if (dist_type == distribution_type::lognormal) {
        parameters.push_back(CalculateMeanLog());
        parameters.push_back(CalculateStdDevLog());
    }

    // CRITICAL: Set the parameters on fitted distribution
    fitted_distribution_.parameters = parameters;

    return parameters;
}

double ConcentrationSet::CalculateLogLikelihood(
    const vector<double>& params,
    distribution_type dist_type) const
{
    double log_likelihood = 0.0;

    // Use fitted distribution if no parameters provided
    if (params.empty() && dist_type == distribution_type::none) {
        for (const double value : *this) {
            log_likelihood += log(fitted_distribution_.Eval(value));
        }
    }
    // Use provided parameters and distribution type
    else if (!params.empty() && dist_type != distribution_type::none) {
        Distribution dist;
        dist.parameters = params;
        dist.distribution = dist_type;

        for (const double value : *this) {
            log_likelihood += log(dist.Eval(value));
        }
    }

    return log_likelihood;
}

distribution_type ConcentrationSet::SelectBestDistribution()
{
    // Lognormal requires all positive values
    if (GetMinimum() <= 0) {
        return distribution_type::normal;
    }

    // For positive data, always use lognormal (based on user's original intent)
    return distribution_type::lognormal;
}

// ========== Goodness-of-Fit Testing ==========

bool ConcentrationSet::CanFitDistribution() const
{
    return size() >= 2 && GetMinimum() < GetMaximum();
}

CMBTimeSeries ConcentrationSet::CreateDataCDF() const
{
    vector<double> sorted_data = QSort(*this);
    CMBTimeSeries cdf;

    double probability_increment = 1.0 / static_cast<double>(sorted_data.size());

    for (size_t i = 0; i < sorted_data.size(); i++) {
        double cumulative_probability = (static_cast<double>(i) + 0.5) * probability_increment;
        cdf.append(sorted_data[i], cumulative_probability);
    }

    return cdf;
}

CMBTimeSeriesSet ConcentrationSet::CreateCDFComparison(distribution_type dist_type) const
{
    CMBTimeSeriesSet comparison;

    // Add empirical CDF
    comparison.append(CreateDataCDF(), "Observed");

    // A zero-width range would never end the loop below
    if (!CanFitDistribution())
        return comparison;

    // Generate fitted CDF
    CMBTimeSeries fitted_cdf;
    double x_min = comparison[0].mint();
    double x_max = comparison[0].maxt();
    double step_size = (x_max - x_min) / 50.0;

    for (double x = x_min; x <= x_max; x += step_size) {
        double cumulative_prob = 0.0;

        if (dist_type == distribution_type::normal) {
            cumulative_prob = gsl_cdf_gaussian_P(x - CalculateMean(), CalculateStdDev());
        }
        else if (dist_type == distribution_type::lognormal) {
            if (x > 0) {
                cumulative_prob = gsl_cdf_lognormal_P(x, CalculateMeanLog(), CalculateStdDevLog());
            }
        }

        fitted_cdf.append(x, cumulative_prob);
    }

    comparison.append(fitted_cdf, "Fitted");
    comparison.append(comparison[0] - comparison[1], "Error");

    return comparison;
}

CMBTimeSeriesSet ConcentrationSet::CreateFittedDistribution(distribution_type dist_type) const
{
    CMBTimeSeriesSet result;

    // Add empirical CDF
    result.append(CreateDataCDF(), "Observed");

    // A zero-width range would never end the loop below
    if (!CanFitDistribution())
        return result;

    // Get distribution parameters
    vector<double> parameters;
    if (dist_type == distribution_type::normal) {
        parameters.push_back(CalculateMean());
        parameters.push_back(CalculateStdDev());
    }
    else if (dist_type == distribution_type::lognormal) {
        parameters.push_back(CalculateMeanLog());
        parameters.push_back(CalculateStdDevLog());
    }

    // Generate fitted PDF
    CMBTimeSeries fitted_pdf;
    double x_min = result[0].mint();
    double x_max = result[0].maxt();
    double step_size = (x_max - x_min) / 50.0;

    for (double x = x_min; x <= x_max; x += step_size) {
        fitted_pdf.append(x, Distribution::Eval(x, parameters, dist_type));
    }

    // Draw the observed values at half the height of the fitted PDF.
    // (Assigning the number to the series itself, as this used to do,
    // converted it through TimeSeries(size_t) and replaced the series with
    // an unnamed empty one, so the later lookup of "Observed" threw.)
    const double observed_height = fitted_pdf.maxC() / 2.0;
    for (auto& point : result["Observed"])
        point.c = observed_height;
    result.append(fitted_pdf, "Fitted");

    return result;
}

double ConcentrationSet::CalculateKolmogorovSmirnovStatistic(
    distribution_type dist_type) const
{
    CMBTimeSeriesSet observed_fitted = CreateCDFComparison(dist_type);

    // No fitted CDF for fewer than two values or constant data
    if (!observed_fitted.Contains("Error"))
        return std::numeric_limits<double>::quiet_NaN();

    // K-S statistic is maximum absolute difference between CDFs
    double max_positive_diff = fabs(observed_fitted[2].maxC());
    double max_negative_diff = fabs(observed_fitted[2].minC());

    return std::max(max_positive_diff, max_negative_diff);
}

// ========== Transformations ==========

double ConcentrationSet::BoxCoxTransformValue(double x, double lambda)
{
    // lambda == 0 is the log limit. Near 0, (x^lambda - 1) cancels and loses
    // digits (the old code switched to log x below 1e-5, an approximation off
    // by about lambda (log x)^2 / 2), so expm1 is used there. Elsewhere the
    // formula is unchanged, which keeps optimal lambdas from moving by
    // round-off: the Kolmogorov-Smirnov grid search is sensitive to it.
    if (lambda == 0.0)
        return std::log(x);
    if (std::fabs(lambda) < 0.01)
        return std::expm1(lambda * std::log(x)) / lambda;
    return (std::pow(x, lambda) - 1.0) / lambda;
}

bool ConcentrationSet::CanBoxCoxTransform() const
{
    return !empty() && GetMinimum() > 0;
}

double ConcentrationSet::CalculateBoxCoxLogLikelihood(double lambda) const
{
    if (!CanBoxCoxTransform())
        return std::numeric_limits<double>::quiet_NaN();

    ConcentrationSet transformed = ApplyBoxCoxTransform(lambda, true);

    double transformed_mean = transformed.CalculateMean();
    double std_dev = CalculateStdDev();

    // The variance term used to be accumulated with a negative sign, so its
    // log was always NaN. (This function has no callers.)
    double variance_term = 0.0;
    double jacobian_term = 0.0;

    for (size_t i = 0; i < size(); i++) {
        variance_term += 1.0 / static_cast<double>(size()) *
            pow(transformed[i] - transformed_mean, 2);
        jacobian_term += (lambda - 1.0) * log(at(i) / std_dev);
    }

    return -static_cast<double>(size()) / 2.0 * log(variance_term) + jacobian_term;
}

ConcentrationSet ConcentrationSet::ApplyBoxCoxTransform(
    double lambda,
    bool normalize) const
{
    // Scale data by standard deviation if normalizing
    ConcentrationSet scaled(size());
    double std_dev = normalize ? CalculateStdDev() : 1.0;

    for (size_t i = 0; i < size(); i++) {
        scaled[i] = at(i) / std_dev;
    }

    // Box-Cox is undefined for zero or negative values (log(0), 0^lambda for
    // lambda < 0): leave the data untransformed. This used to test only for
    // negative values, so a zero produced -inf or inf.
    if (!CanBoxCoxTransform()) {
        return scaled;
    }

    ConcentrationSet transformed(size());

    for (size_t i = 0; i < size(); i++) {
        transformed[i] = BoxCoxTransformValue(scaled[i], lambda);
    }

    return transformed;
}

double ConcentrationSet::FindOptimalBoxCoxParameter(
    double min_lambda,
    double max_lambda,
    int n_intervals) const
{
    // Cannot transform zero or negative values or constant data
    if (!CanBoxCoxTransform() || GetMinimum() == GetMaximum()) {
        return 1.0;
    }

    // Check for NaN values
    if (!(GetMinimum() == GetMinimum())) {
        std::cerr << "Warning: NaN detected in concentration data" << std::endl;
        return 1.0;
    }

    // Base case: range is sufficiently small
    if (fabs(min_lambda - max_lambda) < 1e-6) {
        return (min_lambda + max_lambda) / 2.0;
    }

    // Evaluate K-S statistic at multiple lambda values
    vector<double> ks_statistics;
    double lambda_step = (max_lambda - min_lambda) / static_cast<double>(n_intervals);

    for (double lambda = min_lambda; lambda <= max_lambda; lambda += lambda_step) {
        ConcentrationSet transformed = ApplyBoxCoxTransform(lambda, true);
        double ks_stat = transformed.CalculateKolmogorovSmirnovStatistic(
            distribution_type::normal
        );
        ks_statistics.push_back(ks_stat);
    }

    // Find index of minimum K-S statistic
    int min_index = aquiutils::MinElement(ks_statistics);

    // Recursively refine search around minimum
    int lower_index = std::max(min_index - 1, 0);
    int upper_index = std::min(min_index + 1, static_cast<int>(ks_statistics.size() - 1));

    double refined_min = min_lambda + lower_index * lambda_step;
    double refined_max = min_lambda + upper_index * lambda_step;

    return FindOptimalBoxCoxParameter(refined_min, refined_max, n_intervals);
}

// ========== Utility Functions ==========

vector<unsigned int> ConcentrationSet::CalculateRanks() const
{
    return aquiutils::Rank(*this);
}

