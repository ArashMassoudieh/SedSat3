// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#ifndef CONCENTRATIONSET_H
#define CONCENTRATIONSET_H

#include <vector>
#include <string>
#include "Distribution.h"
#include "cmbtimeseries.h"
#include "cmbtimeseriesset.h"

using namespace std;

/**
 * @class ConcentrationSet
 * @brief Manages a collection of concentration measurements with statistical analysis
 *
 * Inherits from vector<double> to store concentration values and provides:
 * - Statistical measures (mean, standard deviation, norms)
 * - Distribution fitting (normal, lognormal)
 * - Box-Cox transformations
 * - Goodness-of-fit testing (Kolmogorov-Smirnov)
 */
class ConcentrationSet : public vector<double>
{
public:
    // ========== Construction and Assignment ==========

    ConcentrationSet();
    ConcentrationSet(const ConcentrationSet& other);
    ConcentrationSet(int n);
    ConcentrationSet& operator=(const ConcentrationSet& other);

    // ========== Data Management ==========

    /**
     * @brief Append a single concentration value
     */
    void AppendValue(double value);

    /**
     * @brief Append all values from another set
     */
    void AppendSet(const ConcentrationSet& other);
    void AppendSet(ConcentrationSet* other);

    // ========== Basic Statistics ==========

    /**
     * @brief Calculate arithmetic mean
     */
    double CalculateMean() const;

    /**
     * @brief Calculate standard deviation
     * @param mean_value Pre-calculated mean, or -999 to calculate automatically
     */
    double CalculateStdDev(double mean_value = -999) const;

    /**
     * @brief Calculate mean of log-transformed values
     */
    double CalculateMeanLog() const;

    /**
     * @brief Calculate standard deviation of log-transformed values
     * @param mean_value Pre-calculated log mean, or -999 to calculate automatically
     */
    double CalculateStdDevLog(double mean_value = -999) const;

    /**
     * @brief Calculate sum of squared errors from mean
     */
    double CalculateSSE(double mean_value = -999) const;

    /**
     * @brief Calculate sum of squared errors from log mean
     */
    double CalculateSSELog(double mean_value = -999) const;

    /**
     * @brief Calculate sum of values raised to a power
     */
    double CalculateNorm(double power = 2.0) const;

    /**
     * @brief Calculate sum of log-transformed values raised to a power
     */
    double CalculateNormLog(double power = 2.0) const;

    /**
     * @brief Get minimum value
     */
    double GetMinimum() const;

    /**
     * @brief Get maximum value
     */
    double GetMaximum() const;

    /**
     * @brief Whether a distribution can be fitted to the data
     * @return true if there are at least two values and they are not all equal
     *
     * With fewer values, or with constant data, the fitted curves have zero
     * width and the fitting and Kolmogorov-Smirnov routines return only the
     * observed series.
     */
    bool CanFitDistribution() const;

    // ========== Distribution Fitting ==========

    /**
     * @brief Estimate distribution parameters from data
     * @param dist_type Distribution type (normal or lognormal)
     * @return Vector of parameters [mean, std_dev] or [mu, sigma]
     */
    vector<double> EstimateDistributionParameters(
        distribution_type dist_type = distribution_type::none
    );

    /**
     * @brief Calculate log-likelihood of data given distribution parameters
     * @param params Distribution parameters (empty to use fitted_distribution_)
     * @param dist_type Distribution type (none to use fitted_distribution_)
     * @return Log-likelihood value
     */
    double CalculateLogLikelihood(
        const vector<double>& params = vector<double>(),
        distribution_type dist_type = distribution_type::none
    ) const;

    /**
     * @brief Select best-fitting distribution (normal vs lognormal)
     * @return Distribution type with higher log-likelihood
     */
    distribution_type SelectBestDistribution();

    /**
     * @brief Get fitted distribution (from data)
     */
    Distribution* GetFittedDistribution() { return &fitted_distribution_; }
    const Distribution* GetFittedDistribution() const { return &fitted_distribution_; }

    /**
     * @brief Get estimated distribution (for MCMC optimization)
     */
    Distribution* GetEstimatedDistribution() { return &estimated_distribution_; }
    const Distribution* GetEstimatedDistribution() const { return &estimated_distribution_; }

    /**
     * @brief Get/set estimated mu parameter
     */
    double GetEstimatedMu() const { return estimated_distribution_.parameters.size() > 0 ? estimated_distribution_.parameters[0] : 0.0; }
    void SetEstimatedMu(double value) {
        if (estimated_distribution_.parameters.empty()) estimated_distribution_.parameters.resize(2);
        estimated_distribution_.parameters[0] = value;
    }

    /**
     * @brief Get/set estimated sigma parameter
     */
    double GetEstimatedSigma() const { return estimated_distribution_.parameters.size() > 1 ? estimated_distribution_.parameters[1] : 0.0; }
    void SetEstimatedSigma(double value) {
        if (estimated_distribution_.parameters.empty()) estimated_distribution_.parameters.resize(2);
        estimated_distribution_.parameters[1] = value;
    }

    // ========== Goodness-of-Fit Testing ==========

    /**
     * @brief Create empirical CDF from data
     */
    CMBTimeSeries CreateDataCDF() const;

    /**
     * @brief Create comparison of empirical vs fitted CDF
     * @return series named "Observed", "Fitted" and "Error"; only "Observed"
     *         when CanFitDistribution() is false
     */
    CMBTimeSeriesSet CreateCDFComparison(distribution_type dist_type) const;

    /**
     * @brief Create fitted distribution visualization
     * @return series named "Observed" and "Fitted"; only "Observed" when
     *         CanFitDistribution() is false
     */
    CMBTimeSeriesSet CreateFittedDistribution(distribution_type dist_type) const;

    /**
     * @brief Calculate Kolmogorov-Smirnov statistic
     */
    double CalculateKolmogorovSmirnovStatistic(distribution_type dist_type) const;

    // ========== Transformations ==========

    /**
     * @brief Create log-transformed copy
     */
    ConcentrationSet CreateLogTransformed() const;

    /**
     * @brief Box-Cox transform of a single positive value
     *
     * (x^lambda - 1) / lambda; for |lambda| < 0.01 computed as
     * expm1(lambda * log x) / lambda so that it stays accurate near 0;
     * lambda == 0 gives log x, the limiting case. x must be positive.
     */
    static double BoxCoxTransformValue(double x, double lambda);

    /**
     * @brief Whether the Box-Cox transformation applies: all values positive
     *
     * Box-Cox is defined for positive data only. Sets with a zero or negative
     * value are left untransformed by ApplyBoxCoxTransform(), and
     * FindOptimalBoxCoxParameter() returns 1 for them; analyses report which
     * constituents were skipped.
     */
    bool CanBoxCoxTransform() const;

    /**
     * @brief Box-Cox profile log-likelihood for given lambda
     * @return -n/2 log(variance of transformed data) + (lambda - 1) sum log x_i,
     *         or NaN when CanBoxCoxTransform() is false
     */
    double CalculateBoxCoxLogLikelihood(double lambda) const;

    /**
     * @brief Apply Box-Cox transformation
     * @param lambda Transformation parameter
     * @param normalize If true, scale by standard deviation
     * @return transformed values; if CanBoxCoxTransform() is false, the
     *         (scaled) values untransformed
     */
    ConcentrationSet ApplyBoxCoxTransform(double lambda, bool normalize) const;

    /**
     * @brief Find optimal Box-Cox lambda parameter
     * @param min_lambda Lower bound for search
     * @param max_lambda Upper bound for search
     * @param n_intervals Number of intervals for grid search
     * @return Optimal lambda value
     */
    double FindOptimalBoxCoxParameter(
        double min_lambda,
        double max_lambda,
        int n_intervals
    ) const;

    // ========== Utility Functions ==========

    /**
     * @brief Calculate ranks of values (1 = smallest)
     */
    vector<unsigned int> CalculateRanks() const;

private:
    Distribution fitted_distribution_;      ///< Distribution fitted to observed data
    Distribution estimated_distribution_;   ///< Distribution being optimized (e.g., in MCMC)
};

#endif // CONCENTRATIONSET_H
