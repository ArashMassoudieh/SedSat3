#ifndef PARAMETER_H
#define PARAMETER_H

#include "cmbdistribution.h"

/**
 * @enum _range
 * @brief Enumeration for specifying parameter range bounds
 *
 * Used to identify whether referring to the lower or upper bound of a parameter's
 * allowable range in optimization and sampling algorithms.
 */
enum class _range {
    low,   ///< Lower bound of the parameter range
    high   ///< Upper bound of the parameter range
};

/**
 * @class Parameter
 * @brief Represents a model parameter with prior distribution and constraints
 *
 * The Parameter class encapsulates a single model parameter used in statistical
 * inference, optimization, and uncertainty quantification. It stores:
 * - Current parameter value
 * - Allowable range (bounds) for optimization and sampling
 * - Prior probability distribution for Bayesian inference
 * - Parameter name/identifier
 *
 * This class is fundamental to SedSat3's statistical approaches:
 * - **Bayesian MCMC**: Prior distributions guide Markov Chain sampling
 * - **Genetic Algorithms**: Ranges define feasible search space
 * - **Maximum Likelihood Estimation**: Ranges constrain optimization
 *
 * Parameters typically represent source contributions in sediment fingerprinting,
 * with values constrained between 0 and 1 (or 0-100%), and prior distributions
 * reflecting expert knowledge or constraints from the mixing model.
 *
 * ## Prior Distribution Parameterization
 *
 * The class automatically parameterizes prior distributions based on the specified
 * range using UpdatePriorDistribution():
 *
 * - **Normal distribution**: Centered at midpoint with std dev = 0.25 × (high - low)
 *   - μ = 0.5 × (low + high)
 *   - σ = 0.25 × (high - low)
 *
 * - **Lognormal distribution**: Parameters calculated in log-space
 *   - μ_log = 0.5 × (ln(low) + ln(high))
 *   - σ_log = 0.25 × (ln(high) - ln(low))
 *
 * This automatic parameterization ensures that approximately 95% of the prior
 * probability mass falls within the specified range (assuming ±2σ coverage).
 *
 * @note Thread-safe for read-only operations after initialization
 * @warning Changing the range automatically updates prior distribution parameters
 *
 * @see Distribution
 * @see distribution_type
 *
 * Example usage:
 * @code
 * // Create a parameter for agricultural source contribution
 * Parameter sourceContribution;
 * sourceContribution.SetName("Agricultural");
 * sourceContribution.SetRange(0.0, 1.0);  // 0-100% contribution
 * sourceContribution.SetPriorDistribution(distribution_type::normal);
 *
 * // The prior is automatically N(0.5, 0.25²)
 * double logProb = sourceContribution.CalcLogPriorProbability(0.3);
 * @endcode
 */
class Parameter
{
public:
    /**
     * @brief Default constructor
     *
     * Initializes a Parameter object with:
     * - Empty name
     * - Value of 0.0
     * - Range vector with 2 elements (uninitialized bounds)
     * - Default prior distribution (uniform)
     *
     * @post range vector is sized to 2 elements
     */
    Parameter();

    /**
     * @brief Copy constructor
     * @param param The Parameter object to copy from
     *
     * Creates a deep copy of another Parameter, including:
     * - Name
     * - Current value
     * - Range bounds
     * - Complete prior distribution specification
     */
    Parameter(const Parameter &param);

    /**
     * @brief Assignment operator
     * @param param The Parameter object to copy from
     * @return Reference to this object for chaining
     *
     * Copies all data from another Parameter object.
     *
     * @note Does not copy the current value, only structural information
     */
    Parameter& operator=(const Parameter &param);

    /**
     * @brief Set the type of prior probability distribution
     * @param dist_type The distribution type to use
     *
     * Sets the prior distribution type (e.g., normal, lognormal, uniform).
     * Call UpdatePriorDistribution() or SetRange() after this to parameterize
     * the distribution based on the current range.
     *
     * @see distribution_type
     * @see UpdatePriorDistribution()
     *
     * Example:
     * @code
     * param.SetPriorDistribution(distribution_type::lognormal);
     * param.SetRange(0.01, 10.0);  // Automatically parameterizes lognormal
     * @endcode
     */
    void SetPriorDistribution(distribution_type dist_type) {
        prior_distribution.distribution = dist_type;
    }

    /**
     * @brief Get reference to the prior distribution object
     * @return Reference to the internal Distribution object
     *
     * Provides direct access to the prior distribution for:
     * - Querying distribution parameters
     * - Evaluating probability densities
     * - Sampling from the distribution
     *
     * @warning Modifying the returned distribution directly may lead to
     *          inconsistency with the parameter range
     */
    Distribution& GetPriorDistribution() {
        return prior_distribution;
    }

    /**
     * @brief Compare parameter with a string (unused, legacy method)
     * @param dist_type String to compare with
     * @return Comparison result
     *
     * @deprecated This method appears to be unused and may be removed
     */
    bool operator==(const string &dist_type);

    /**
     * @brief Get range bound by string identifier
     * @param quantity String specifying which bound: "high" or "low"
     * @return The requested range bound, or 0 if quantity not recognized
     *
     * Legacy method providing string-based access to range bounds.
     * Prefer using GetRange(_range) for type-safe access.
     *
     * @see GetRange(_range)
     */
    double GetVal(const string &quantity);

    /**
     * @brief Set the parameter range from a vector
     * @param rng Vector containing [low_bound, high_bound]
     *
     * Sets both range bounds from a 2-element vector and automatically updates
     * the prior distribution parameters to match the new range.
     *
     * @pre rng.size() must equal 2
     * @post Prior distribution is parameterized based on new range
     *
     * @see UpdatePriorDistribution()
     */
    void SetRange(const vector<double> &rng);

    /**
     * @brief Set the parameter range from individual bounds
     * @param low Lower bound of the allowable range
     * @param high Upper bound of the allowable range
     *
     * Sets the parameter's feasible range and automatically updates the prior
     * distribution parameters. This is the preferred method for setting ranges.
     *
     * @post Prior distribution is parameterized based on new range
     * @post range vector is resized to 2 elements if needed
     *
     * Example:
     * @code
     * param.SetRange(0.0, 1.0);  // Source contribution: 0-100%
     * @endcode
     */
    void SetRange(double low, double high);

    /**
     * @brief Get a specific range bound
     * @param lowhigh Specifies which bound to retrieve
     * @return The requested bound value, or 0 if range is not properly initialized
     *
     * Type-safe method for retrieving range bounds.
     *
     * @pre range vector must be properly initialized (size == 2)
     */
    double GetRange(_range lowhigh);

    /**
     * @brief Set a specific range bound
     * @param lowhigh Specifies which bound to set (low or high)
     * @param value The new bound value
     *
     * Sets one bound of the range while leaving the other unchanged. Automatically
     * updates the prior distribution parameters to reflect the new range.
     *
     * @post Prior distribution is parameterized based on new range
     * @post range vector is resized to 2 elements if needed
     *
     * Example:
     * @code
     * param.SetRange(_range::low, 0.05);   // Set minimum to 5%
     * param.SetRange(_range::high, 0.95);  // Set maximum to 95%
     * @endcode
     */
    void SetRange(_range, double value);

    /**
     * @brief Set the parameter name/identifier
     * @param nam The parameter name (e.g., "Agricultural_Contribution", "Forest_Source")
     *
     * Parameter names are used for:
     * - Display in GUI and reports
     * - Result file headers
     * - Identifying parameters in optimization output
     */
    void SetName(const string &nam) {
        name = nam;
    }

    /**
     * @brief Get the parameter name
     * @return The parameter's name/identifier
     */
    string Name() const {
        return name;
    }

    /**
     * @brief Get the current parameter value
     * @return The current value
     *
     * @see GetValue()
     */
    double Value() const {
        return value;
    }

    /**
     * @brief Set the current parameter value
     * @param val The new value
     *
     * Sets the parameter's current value, typically during:
     * - MCMC sampling (each iteration)
     * - Genetic algorithm evolution
     * - Optimization convergence
     *
     * @note Does not validate that val is within the specified range
     * @warning Value may be outside range during optimization algorithms
     */
    void SetValue(double val) {
        value = val;
    }

    /**
     * @brief Get the current parameter value (alternative accessor)
     * @return The current value
     *
     * @see Value()
     */
    double GetValue() {
        return value;
    }

    /**
     * @brief Update prior distribution parameters based on current range
     *
     * Automatically parameterizes the prior distribution to be consistent with
     * the specified range. Called automatically by SetRange methods.
     *
     * ## Parameterization Schemes:
     *
     * **Normal Distribution:**
     * - Mean (μ) = midpoint of range = 0.5 × (low + high)
     * - Std dev (σ) = 0.25 × (high - low)
     * - Results in ~95% probability within [low, high] (±2σ)
     *
     * **Lognormal Distribution:**
     * - Log-space mean (μ_log) = 0.5 × (ln(low) + ln(high))
     * - Log-space std dev (σ_log) = 0.25 × (ln(high) - ln(low))
     * - Appropriate for strictly positive parameters with multiplicative uncertainty
     *
     * **Other Distributions:**
     * - No automatic parameterization (parameters unchanged)
     *
     * @note Only affects normal and lognormal distributions
     * @pre range vector must be properly initialized with 2 elements
     * @post prior_distribution.parameters vector is updated
     *
     * @see SetRange(double, double)
     */
    void UpdatePriorDistribution();

    /**
     * @brief Calculate log prior probability density at a given value
     * @param x The value at which to evaluate the log prior probability
     * @return Natural logarithm of the prior probability density at x
     *
     * Evaluates ln(p(x)) where p is the prior probability density function.
     * Used in Bayesian inference to calculate:
     * - Log posterior probability: ln(p(θ|data)) = ln(p(data|θ)) + ln(p(θ))
     * - Metropolis-Hastings acceptance ratios
     * - Maximum a posteriori (MAP) estimates
     *
     * Using log probabilities prevents numerical underflow and is computationally
     * more stable for MCMC sampling.
     *
     * @note Returns -∞ (or very large negative value) if x is outside support of distribution
     * @see Distribution::EvalLog()
     *
     * Example:
     * @code
     * Parameter source;
     * source.SetRange(0.0, 1.0);
     * source.SetPriorDistribution(distribution_type::normal);
     *
     * double logPrior = source.CalcLogPriorProbability(0.5);  // Highest at mean
     * double logPrior2 = source.CalcLogPriorProbability(0.95); // Lower at tail
     * @endcode
     */
    double CalcLogPriorProbability(const double x);

private:
    /**
     * @brief The prior probability distribution for this parameter
     *
     * Encodes prior knowledge or beliefs about the parameter's likely values
     * before observing data. Used in Bayesian inference to constrain the
     * posterior distribution.
     *
     * The distribution is automatically parameterized by UpdatePriorDistribution()
     * based on the specified range for normal and lognormal distributions.
     */
    Distribution prior_distribution;

    /**
     * @brief Allowable range for the parameter [low, high]
     *
     * Two-element vector defining the feasible region for optimization and sampling.
     * For source contributions, typically [0.0, 1.0] or [0.0, 100.0].
     *
     * Range bounds serve multiple purposes:
     * - Hard constraints in optimization algorithms
     * - Basis for automatic prior distribution parameterization
     * - Validity checking in Bayesian inference
     *
     * @invariant range.size() == 2
     * @invariant range[0] < range[1] (low < high)
     */
    vector<double> range;

    /**
     * @brief Parameter name/identifier
     *
     * Human-readable name used for display, reporting, and identification.
     * Examples: "Agricultural", "Forest", "Urban", "Geological_Baseline"
     */
    string name;

    /**
     * @brief Current value of the parameter
     *
     * The current estimate or sample value. Updated during:
     * - MCMC sampling (each iteration)
     * - Genetic algorithm evolution (each generation)
     * - Optimization (each step toward minimum)
     *
     * Default value is 0.0 after construction.
     */
    double value = 0;
};

#endif // PARAMETER_H
