#ifndef CMBDISTRIBUTION_H
#define CMBDISTRIBUTION_H

#include <vector>
#include "TimeSeries.h"

/**
 * @enum distribution_type
 * @brief Enumeration of probability distribution types supported in SedSat3
 *
 * Defines the statistical distributions available for modeling uncertainty in
 * tracer concentrations, parameter priors, and observational errors.
 */
enum class distribution_type {
    normal,      ///< Normal (Gaussian) distribution: p(x) = N(μ, σ²)
    lognormal,   ///< Lognormal distribution: ln(x) ~ N(μ, σ²), for strictly positive variables
    dirichlet,   ///< Dirichlet distribution (treated as uniform on simplex in current implementation)
    none,        ///< No distribution assigned (uninitialized state)
    uniform      ///< Uniform distribution on [0,1]
};

/**
 * @enum parameter_mode
 * @brief Specifies whether to use direct data statistics or fitted distribution parameters
 *
 * Controls how mean/variance are calculated from a Distribution object:
 * - **direct**: Use empirical statistics from original data (mean_val, std_val)
 * - **based_on_fitted_distribution**: Calculate from distribution parameters
 */
enum class parameter_mode {
    direct,                          ///< Use empirical statistics from data
    based_on_fitted_distribution     ///< Calculate from fitted distribution parameters
};

/**
 * @class Distribution
 * @brief Represents a parametric probability distribution for uncertainty quantification
 *
 * The Distribution class provides:
 * - Probability density evaluation (PDF)
 * - Log-probability evaluation (for numerical stability in Bayesian inference)
 * - Distribution visualization
 * - Parameter storage for various distribution types
 * - Conversion between data statistics and distribution parameters
 *
 * ## Supported Distributions
 *
 * ### Normal Distribution
 * - **Parameters**: [μ, σ] where μ is mean, σ is standard deviation
 * - **PDF**: p(x) = (1/(√(2π)σ)) × exp(-(x-μ)²/(2σ²))
 * - **Support**: (-∞, +∞)
 * - **Use case**: Tracer concentrations with symmetric uncertainty, isotopic ratios
 *
 * ### Lognormal Distribution
 * - **Parameters**: [μ_log, σ_log] in log-space
 * - **PDF**: p(x) = (1/(x√(2π)σ_log)) × exp(-(ln(x)-μ_log)²/(2σ_log²))
 * - **Support**: (0, +∞)
 * - **Use case**: Elemental concentrations (strictly positive), ratios
 * - **Mean**: exp(μ_log + σ_log²/2)
 *
 * ### Uniform/Dirichlet Distribution
 * - **Parameters**: None (implicitly [0,1])
 * - **PDF**: p(x) = 1 for x ∈ [0,1], else very small (1e-32)
 * - **Support**: [0, 1]
 * - **Use case**: Non-informative priors for source contributions
 *
 * ## Usage in SedSat3
 *
 * This class is central to:
 * 1. **Prior distributions** (Parameter class) for Bayesian inference
 * 2. **Measurement uncertainty** (ConcentrationSet) for error propagation
 * 3. **Fitted distributions** for tracer selection (normality tests)
 *
 * @note Thread-safe for const operations after initialization
 * @see Parameter
 * @see ConcentrationSet
 *
 * Example usage:
 * @code
 * // Create a normal distribution for a tracer concentration
 * Distribution tracerDist;
 * tracerDist.SetType(distribution_type::normal);
 * tracerDist.parameters = {50.0, 5.0};  // μ=50 mg/kg, σ=5 mg/kg
 *
 * // Evaluate probability density
 * double pdf = tracerDist.Eval(55.0);
 *
 * // Evaluate log-probability (preferred for Bayesian calculations)
 * double logPdf = tracerDist.EvalLog(55.0);
 *
 * // Generate visualization data
 * TimeSeries<double> curve = tracerDist.EvaluateAsTimeSeries(200, 3.0);
 * @endcode
 */
class Distribution
{
public:
    /**
     * @brief Default constructor
     *
     * Initializes a Distribution with:
     * - distribution = distribution_type::none
     * - Empty parameters vector
     * - Zero mean and standard deviation
     *
     * @post pi is initialized to π (3.14159...)
     */
    Distribution();

    /**
     * @brief Copy constructor
     * @param dist The Distribution object to copy from
     *
     * Creates a deep copy including:
     * - Distribution type
     * - All parameters
     * - Empirical statistics (mean_val, std_val)
     */
    Distribution(const Distribution &dist);

    /**
     * @brief Assignment operator
     * @param dist The Distribution object to copy from
     * @return Reference to this object for chaining
     *
     * Copies all distribution data and reinitializes pi constant.
     */
    Distribution& operator = (const Distribution &dist);

    /**
     * @brief Evaluate probability density function at a given value
     * @param x The value at which to evaluate the PDF
     * @return The probability density p(x)
     *
     * Computes the probability density function value. For continuous distributions,
     * this is NOT a probability (which would require integration), but the density
     * at point x.
     *
     * ## Mathematical Formulas:
     *
     * **Normal**: p(x) = (1/(√(2π)σ)) × exp(-(x-μ)²/(2σ²))
     *
     * **Lognormal**: p(x) = (1/(x√(2π)σ_log)) × exp(-(ln(x)-μ_log)²/(2σ_log²))
     *
     * **Uniform/Dirichlet**: p(x) = 1 for x ∈ [0,1], else 1×10⁻³²
     *
     * @note For numerical reasons, uniform distributions return 1e-32 (not exactly 0) outside [0,1]
     * @note Prefer EvalLog() for Bayesian calculations to avoid underflow
     *
     * @see EvalLog()
     */
    double Eval(const double &x) const;

    /**
     * @brief Static method to evaluate PDF with explicit parameters
     * @param x The value at which to evaluate
     * @param parameters Distribution parameters [μ, σ] or [μ_log, σ_log]
     * @param distribution The distribution type
     * @return The probability density p(x)
     *
     * Utility function for evaluating PDFs without creating a Distribution object.
     * Useful for one-off calculations or testing.
     *
     * Example:
     * @code
     * vector<double> params = {0.0, 1.0};  // Standard normal
     * double pdf = Distribution::Eval(1.96, params, distribution_type::normal);
     * // pdf ≈ 0.0584 (height of N(0,1) at 1.96σ)
     * @endcode
     */
    static double Eval(const double &x, const vector<double> parameters, distribution_type distribution);

    /**
     * @brief Evaluate natural logarithm of probability density
     * @param x The value at which to evaluate
     * @return ln(p(x)) - natural log of probability density
     *
     * Computes the log-probability density, which is numerically more stable than
     * Eval() for Bayesian inference. This prevents underflow when multiplying many
     * small probabilities and is faster (avoids division and some exponentials).
     *
     * ## Mathematical Formulas:
     *
     * **Normal**:
     * ln(p(x)) = -ln(√(2π)σ) - (x-μ)²/(2σ²)
     *
     * **Lognormal**:
     * ln(p(x)) = -ln(√(2π)σ_log×x) - (ln(x)-μ_log)²/(2σ_log²)
     *
     * **Uniform/Dirichlet**:
     * ln(p(x)) = 0 for x ∈ [0,1], else -1×10⁶ (effectively -∞)
     *
     * ## Usage in Bayesian Inference:
     *
     * Log-posterior = log-likelihood + log-prior:
     * @code
     * double logPosterior = logLikelihood + prior.EvalLog(theta);
     * @endcode
     *
     * @note Returns large negative value (-1e6) instead of -∞ for numerical stability
     * @note This is the preferred method for MCMC and optimization
     *
     * @see CalcLogPriorProbability()
     */
    double EvalLog(const double &x);

    /**
     * @brief Generate time series representation of the distribution for plotting
     * @param numberofpoint Number of points to generate (default: 100)
     * @param stdcoeff Number of standard deviations to span (default: 4, i.e., ±4σ)
     * @return TimeSeries containing (x, p(x)) pairs for visualization
     *
     * Creates a discrete representation of the PDF suitable for plotting in GUI.
     * The x-values span from (μ - stdcoeff×σ) to (μ + stdcoeff×σ) for normal
     * distributions, or the equivalent in log-space for lognormal distributions.
     *
     * ## Coverage by stdcoeff:
     * - stdcoeff = 1: ~68% of probability mass (±1σ)
     * - stdcoeff = 2: ~95% of probability mass (±2σ)
     * - stdcoeff = 3: ~99.7% of probability mass (±3σ)
     * - stdcoeff = 4: ~99.99% of probability mass (±4σ, default)
     *
     * @note For lognormal, x values are exp(μ_log ± stdcoeff×σ_log)
     * @note The returned TimeSeries can be directly plotted or exported
     *
     * Example:
     * @code
     * Distribution dist;
     * dist.SetType(distribution_type::normal);
     * dist.parameters = {100.0, 10.0};
     *
     * // Generate curve spanning ±3σ (99.7% of mass)
     * TimeSeries<double> curve = dist.EvaluateAsTimeSeries(200, 3.0);
     * // curve contains 200 points from x=70 to x=130
     * @endcode
     */
    TimeSeries<double> EvaluateAsTimeSeries(int numberofpoint=100, const double &stdcoeff = 4);

    /**
     * @brief Static method to generate time series with explicit parameters
     * @param numberofpoints Number of points to generate
     * @param stdcoeff Number of standard deviations to span
     * @param parameters Distribution parameters [μ, σ] or [μ_log, σ_log]
     * @param dist_type The distribution type
     * @return TimeSeries containing (x, p(x)) pairs
     *
     * Static version of EvaluateAsTimeSeries() for generating curves without
     * creating a Distribution object.
     *
     * @see EvaluateAsTimeSeries(int, const double&)
     */
    static TimeSeries<double> EvaluateAsTimeSeries(int numberofpoints, const double &stdcoeff,
                                                   const vector<double> parameters,
                                                   distribution_type &dist_type);

    /**
     * @brief Distribution parameters vector
     *
     * Storage for distribution-specific parameters:
     * - **Normal**: parameters[0] = μ (mean), parameters[1] = σ (std dev)
     * - **Lognormal**: parameters[0] = μ_log, parameters[1] = σ_log
     * - **Uniform/Dirichlet**: Empty or unused
     *
     * @note Size is automatically adjusted by SetType()
     * @note Direct public access provided for flexibility
     */
    vector<double> parameters;

    /**
     * @brief The type of probability distribution
     *
     * Determines which PDF formula to use in Eval() and EvalLog().
     * Default is distribution_type::none (uninitialized).
     */
    distribution_type distribution = distribution_type::none;

    /**
     * @brief Set the distribution type and resize parameters vector
     * @param typ The distribution type to set
     *
     * Sets the distribution type and automatically resizes the parameters vector:
     * - Normal → 2 parameters [μ, σ]
     * - Lognormal → 2 parameters [μ_log, σ_log]
     * - Others → no automatic resize
     *
     * @post parameters vector is appropriately sized
     * @note Does not initialize parameter values, only sizes the vector
     */
    void SetType(const distribution_type &typ);

    /**
     * @brief Calculate the mean (expected value) of the distribution
     * @param param_mode Whether to use empirical data or distribution parameters
     * @return The mean value E[X]
     *
     * Computes the expected value based on the selected mode:
     *
     * **direct mode**: Returns the stored empirical mean (mean_val) from original data
     *
     * **based_on_fitted_distribution mode** (default):
     * - **Normal**: μ (parameters[0])
     * - **Lognormal**: exp(μ_log + σ_log²/2) (accounting for log-space variance)
     * - **Others**: 0
     *
     * @note For lognormal, the mean in real space is NOT simply exp(μ_log)
     * @see SetDataMean()
     */
    double Mean(parameter_mode param_mode = parameter_mode::based_on_fitted_distribution);

    /**
     * @brief Set the empirical mean from original data
     * @param val The empirical mean value
     *
     * Stores the sample mean calculated directly from observed data,
     * independent of the fitted distribution parameters.
     */
    void SetDataMean(const double &val) {
        mean_val = val;
    }

    /**
     * @brief Set the empirical standard deviation from original data
     * @param val The empirical standard deviation
     *
     * Stores the sample standard deviation calculated directly from observed data,
     * independent of the fitted distribution parameters.
     */
    void SetDataSTDev(const double &val) {
        std_val = val;
    }

    /**
     * @brief Get the stored empirical mean
     * @return The empirical mean from original data
     */
    double DataMean() {
        return mean_val;
    }

    /**
     * @brief Get the stored empirical standard deviation
     * @return The empirical standard deviation from original data
     */
    double DataSTDev() {
        return std_val;
    }

    /**
     * @brief Compare distribution type with a string identifier
     * @param dist_type String name of distribution ("normal", "log-normal", "uniform", "dirichlet")
     * @return true if distribution matches the string type
     *
     * Convenience method for string-based type checking, useful for
     * configuration file parsing or user input validation.
     *
     * Recognized strings:
     * - "normal" → distribution_type::normal
     * - "log-normal" → distribution_type::lognormal
     * - "uniform" → distribution_type::uniform or dirichlet
     * - "dirichlet" → distribution_type::dirichlet
     *
     * Example:
     * @code
     * if (dist == "normal") {
     *     // Handle normal distribution case
     * }
     * @endcode
     */
    bool operator==(const string &dist_type);

private:
    /**
     * @brief Mathematical constant π (pi)
     *
     * Computed once as 4×arctan(1) ≈ 3.14159265359
     * Used in normal and lognormal PDF calculations.
     *
     * @note Static member shared across all Distribution instances
     */
    static double pi;

    /**
     * @brief Empirical mean from original sample data
     *
     * Stores the sample mean calculated directly from observed tracer concentrations
     * or other measured data. This is independent of any fitted distribution parameters
     * and represents the actual data statistics.
     *
     * Used when param_mode::direct is specified in Mean().
     */
    double mean_val = 0;

    /**
     * @brief Empirical standard deviation from original sample data
     *
     * Stores the sample standard deviation calculated directly from observations.
     * Represents actual data variability independent of fitted distribution.
     */
    double std_val = 0;
};

#endif // CMBDISTRIBUTION_H
