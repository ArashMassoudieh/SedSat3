#ifndef CGACLASS
#define CGACLASS
#pragma once

#include "Individual.h"
#include "GADistribution.h"
#include "math.h"
#include <string>
#include <vector>
#include <map>

#ifndef mac_version
#endif

// Forward declaration for GUI support
class ProgressWindow;

/**
 * @struct GA_Tweaking_parameters
 * @brief Configuration parameters for Genetic Algorithm optimization
 *
 * Contains all tuning parameters and settings that control the behavior of
 * the genetic algorithm, including population size, genetic operators,
 * and convergence criteria.
 */
struct GA_Tweaking_parameters
{
    /**
     * @brief Total number of parameters in the optimization problem
     *
     * Dimensionality of the parameter space. For sediment fingerprinting,
     * typically equals the number of sources to be apportioned.
     */
    int totnumparams;

    /**
     * @brief Maximum population size (number of individuals)
     *
     * Number of candidate solutions maintained in each generation.
     * Larger populations explore more broadly but require more evaluations.
     *
     * Typical: 50-200 individuals
     * Default: 2 (likely intended as multiplier, not actual size)
     */
    int maxpop = 2;

    /**
     * @brief Number of parameters (may be subset of totnumparams)
     *
     * Number of active parameters being optimized (non-fixed parameters).
     */
    int nParam;

    /**
     * @brief Number of enhancement iterations
     *
     * Additional optimization refinements applied to best solutions.
     */
    int numenhancements;

    /**
     * @brief Current enhancement iteration counter
     */
    int num_enh;

    /**
     * @brief Number of generations to evolve
     *
     * Total iterations of selection, crossover, and mutation.
     * More generations allow better convergence but take longer.
     *
     * Typical: 100-1000 generations
     */
    int nGen;

    /**
     * @brief Crossover operator type
     *
     * Selects which crossover strategy to use for combining parent solutions:
     * - Type 0: Single-point crossover
     * - Type 1: Two-point crossover
     * - Type 2: Uniform crossover
     * - Other: Problem-specific operators
     */
    int cross_over_type;

    /**
     * @brief Number of bins for fitness distribution
     *
     * Used in fitness proportionate selection to discretize fitness values
     * for roulette wheel selection.
     */
    int no_bins;

    /**
     * @brief Output sensitivity analysis results
     *
     * If true, calculates and saves parameter sensitivity information
     * during optimization.
     *
     * Default: false
     */
    bool sens_out = false;

    /**
     * @brief Use Real-Coded Genetic Algorithm
     *
     * If true, uses continuous (real-valued) representation and operators.
     * If false, uses binary encoding and operators.
     *
     * RCGA is generally superior for continuous optimization problems.
     *
     * Default: false
     */
    bool RCGA = false;

    /**
     * @brief Read initial population from file
     *
     * If true, loads starting population from file instead of random initialization.
     * Useful for warm-starting optimization or continuing previous runs.
     *
     * Default: false
     */
    bool readfromgafile;

    /**
     * @brief Apply steepest descent local search
     *
     * If true, applies gradient-based refinement to best individuals
     * to accelerate convergence near optima. Creates hybrid GA/local-search algorithm.
     *
     * Default: false
     */
    bool Steepest_Descent = false;

    /**
     * @brief Selection pressure parameter for rank-based selection
     *
     * Controls intensity of selection pressure in rank-based fitness assignment.
     * Higher N gives stronger preference to better individuals.
     *
     * Typical: 1.5-2.0
     */
    double N;

    /**
     * @brief Crossover probability
     *
     * Probability that two selected parents undergo crossover.
     * If crossover doesn't occur, parents are copied to offspring unchanged.
     *
     * Typical: 0.6-0.9
     * Default: 1.0 (always crossover)
     */
    double pcross = 1;

    /**
     * @brief Mutation probability per gene
     *
     * Probability that each parameter value is randomly perturbed.
     * Maintains diversity and enables exploration of new regions.
     *
     * Typical: 0.001-0.01 for binary, 0.01-0.1 for real-coded
     * Default: 0.02 (2%)
     */
    double pmute = 0.02;

    /**
     * @brief Exponent coefficient for fitness scaling
     *
     * Used in exponential fitness transformation to control selection pressure:
     * scaled_fitness = exp(exponentcoeff × raw_fitness)
     *
     * Default: 1.0
     */
    double exponentcoeff = 1;

    /**
     * @brief Scale factor for shake/perturbation operation
     *
     * Controls magnitude of random perturbations in shake operation,
     * specified as fraction of parameter range.
     *
     * Default: 0.05 (5% of range)
     */
    double shakescale = 0.05;

    /**
     * @brief Reduction factor for shake scale over generations
     *
     * Shake scale is multiplied by this factor each generation to
     * gradually reduce perturbation magnitude (simulated annealing effect).
     *
     * Default: 0.75 (25% reduction per generation)
     */
    double shakescalered = 0.75;

    /**
     * @brief Fitness function type identifier
     *
     * Character code specifying which objective function to use:
     * - 'L': Least squares error
     * - 'M': Maximum likelihood
     * - 'C': Custom fitness function
     * - Other: Problem-specific functions
     */
    char fitnesstype;
};

/**
 * @struct _filenames
 * @brief File paths for Genetic Algorithm input/output operations
 *
 * Contains all file paths needed for GA initialization, checkpointing,
 * and result output.
 */
struct _filenames
{
    /**
     * @brief Path to file containing initial population
     *
     * Used when readfromgafile is true to load starting population.
     */
    std::string initialpopfilemame;

    /**
     * @brief Directory path for output files
     *
     * Base directory where all GA results will be saved.
     */
    std::string pathname;

    /**
     * @brief Path to file for reading previous GA results
     *
     * Used to warm-start optimization from previous run.
     */
    std::string getfromfilename;

    /**
     * @brief Path to main output file for GA results
     *
     * Contains:
     * - Best individual each generation
     * - Population statistics
     * - Convergence history
     */
    std::string outputfilename;
};

/**
 * @class CGA
 * @brief Genetic Algorithm optimizer for global parameter estimation
 *
 * The CGA class implements a sophisticated genetic algorithm with both binary
 * and real-coded representations for solving complex optimization problems in
 * sediment source fingerprinting.
 *
 * ## Algorithm Overview
 *
 * Genetic Algorithms are population-based metaheuristic optimization methods
 * inspired by natural evolution:
 *
 * 1. **Initialize**: Create random population of candidate solutions
 * 2. **Evaluate**: Calculate fitness (objective function) for each individual
 * 3. **Select**: Choose parents based on fitness (better individuals more likely)
 * 4. **Crossover**: Combine parent solutions to create offspring
 * 5. **Mutate**: Randomly perturb offspring to maintain diversity
 * 6. **Replace**: Form next generation from offspring (and/or parents)
 * 7. **Repeat**: Continue until convergence or generation limit reached
 *
 * ## Key Features
 *
 * - **Multiple encoding schemes**: Binary and real-coded representations
 * - **Flexible selection methods**: Fitness proportionate, rank-based, tournament
 * - **Various crossover operators**: Single-point, two-point, uniform, arithmetic
 * - **Adaptive mutation**: Self-adjusting based on convergence state
 * - **Hybrid optimization**: Optional gradient-based local search
 * - **Parallel evaluation**: Thread-safe fitness evaluation
 * - **Progress tracking**: GUI integration for real-time monitoring
 *
 * ## Usage in SedSat3
 *
 * GA provides global optimization for source apportionment when:
 * - Posterior is multimodal (multiple local optima)
 * - Gradient information unavailable or unreliable
 * - Robust global search needed
 * - Derivative-free optimization preferred
 *
 * GA finds parameter values that minimize error between predicted and observed
 * tracer concentrations, subject to constraints (e.g., contributions sum to 100%).
 *
 * ## Mathematical Details
 *
 * **Fitness function** (for minimization):
 * @code
 * fitness = -Σ((y_observed - y_predicted)² / σ²)
 * @endcode
 *
 * **Selection probability** (fitness proportionate):
 * @code
 * P(select individual i) = fitness_i / Σ(fitness_j)
 * @endcode
 *
 * **Rank-based fitness**:
 * @code
 * fitness_rank(i) = N - 2(N-1)(rank(i)-1)/(population_size-1)
 * @endcode
 * where N is selection pressure parameter (typically 1.5-2.0)
 *
 * @tparam T Model class type (e.g., SourceSinkData) that provides:
 *           - Parameter setting interface
 *           - Objective function evaluation
 *           - Model predictions
 *
 * @note Thread-safe for parallel fitness evaluation
 * @warning May converge to local optimum; multiple runs recommended
 *
 * @see CIndividual
 * @see GADistribution
 * @see SourceSinkData
 *
 * Example usage:
 * @code
 * // Create GA optimizer for source apportionment
 * CGA<SourceSinkData> ga(&sourceData);
 *
 * // Configure GA parameters
 * ga.GA_params.maxpop = 100;
 * ga.GA_params.nGen = 500;
 * ga.GA_params.pcross = 0.8;
 * ga.GA_params.pmute = 0.02;
 * ga.GA_params.RCGA = true;  // Use real-coded GA
 *
 * // Set parameter ranges
 * ga.minval = {0.0, 0.0, 0.0};  // 3 sources
 * ga.maxval = {1.0, 1.0, 1.0};
 *
 * // Initialize and run
 * ga.InitiatePopulation();
 * ga.optimize();
 *
 * // Extract best solution
 * std::vector<double> bestParams = ga.final_params;
 * @endcode
 */
template<class T>
class CGA
{
public:
    /**
     * @brief Sum of all fitness values in current population
     *
     * Used for fitness proportionate selection (roulette wheel).
     * Updated after each fitness evaluation.
     */
    double sumfitness;

    /**
     * @brief Maximum fitness value in current population
     *
     * Tracks the best individual's fitness for convergence monitoring
     * and termination criteria.
     */
    double MaxFitness;

    /**
     * @brief GA algorithm configuration parameters
     *
     * All tuning parameters controlling GA behavior.
     * Modify before calling initialize() or optimize().
     */
    GA_Tweaking_parameters GA_params;

    /**
     * @brief File paths for GA input/output
     *
     * Configuration structure containing paths for initialization files,
     * results output, and checkpoint files.
     */
    _filenames filenames;

    /**
     * @brief Percentiles to calculate for output distributions
     *
     * Vector of percentile values (0-1 scale) to compute from
     * population of solutions.
     * Typical: {0.025, 0.5, 0.975} for median and 95% range
     */
    std::vector<double> calc_output_percentiles;

    /**
     * @brief Initial population loaded from file
     *
     * Parameter values for starting population when readfromgafile is true.
     * Structure: initial_pop[individual][parameter]
     */
    std::vector<std::vector<double>> initial_pop;

    /**
     * @brief Final optimized parameter values
     *
     * Best solution found by GA (parameter vector of best individual
     * from final generation).
     */
    std::vector<double> final_params;

    /**
     * @brief Parameter indices being optimized
     *
     * Maps GA parameter space to model parameter space.
     * Allows optimizing subset of parameters while fixing others.
     */
    std::vector<int> params;

    /**
     * @brief Flags indicating if parameters are log-transformed
     *
     * If loged[i] is true, parameter i is optimized in log-space.
     * Useful for parameters spanning multiple orders of magnitude.
     */
    std::vector<int> loged;

    /**
     * @brief Time series index for each parameter (if applicable)
     *
     * Maps parameters to specific time series or observations.
     */
    std::vector<int> to_ts;

    /**
     * @brief Fixed values for non-optimized parameters
     *
     * Parameters not being optimized are held constant at these values.
     */
    std::vector<double> fixedinputvale;

    /**
     * @brief Lower bounds for each parameter
     *
     * Minimum allowable values defining search space boundaries.
     * For source contributions: typically 0.0
     */
    std::vector<double> minval;

    /**
     * @brief Upper bounds for each parameter
     *
     * Maximum allowable values defining search space boundaries.
     * For source contributions: typically 1.0 or 100.0
     */
    std::vector<double> maxval;

    /**
     * @brief Flags indicating if parameter applies to all entities
     *
     * Used for shared parameters across multiple sources or observations.
     */
    std::vector<bool> apply_to_all;

    /**
     * @brief Output comparison indices (legacy, may be unused)
     *
     * Mapping for comparing model outputs to observations.
     */
    std::vector<std::vector<int>> outcompare;

    /**
     * @brief Current population of individuals
     *
     * Active population of candidate solutions being evolved.
     * Size: GA_params.maxpop
     */
    std::vector<CIndividual> Ind;

    /**
     * @brief Previous generation's population
     *
     * Backup of population from previous generation for comparison
     * and elitism strategies.
     */
    std::vector<CIndividual> Ind_old;

    /**
     * @brief Names of parameters being optimized
     *
     * Human-readable labels for each parameter (e.g., "Agricultural",
     * "Forest", "Urban") used in output files and visualization.
     */
    std::vector<std::string> paramname;

    /**
     * @brief Copies of model for parallel fitness evaluation
     *
     * Each individual gets its own model copy for thread-safe evaluation.
     * Size: GA_params.maxpop
     */
    std::vector<T> Models;

    /**
     * @brief Output model configured with best parameters
     *
     * Model instance set to optimal parameter values after optimization.
     */
    T Model_out;

    /**
     * @brief Pointer to the model being optimized
     *
     * Reference to model that provides fitness evaluation.
     */
    T *Model;

    /**
     * @brief Default constructor
     *
     * Creates uninitialized GA object. Must call other initialization
     * methods before running.
     */
    CGA();

    /**
     * @brief Destructor
     *
     * Cleans up allocated memory and closes output files.
     */
    virtual ~CGA();

    /**
     * @brief Copy constructor
     * @param C The CGA object to copy from
     *
     * Creates deep copy including population, settings, and model copies.
     */
    CGA(const CGA &C);

    /**
     * @brief Assignment operator
     * @param C The CGA object to assign from
     * @return Reference to this object
     *
     * Copies all data from another GA object.
     */
    CGA operator=(CGA &C);

    /**
     * @brief Constructor from configuration file and model
     * @param filename Path to GA configuration file
     * @param model Reference to model to optimize
     *
     * Initializes GA from configuration file containing parameters,
     * ranges, and algorithm settings.
     */
    CGA(std::string filename, const T& model);

    /**
     * @brief Constructor from model pointer
     * @param model Pointer to model to optimize
     *
     * Preferred constructor linking GA to specific model instance.
     */
    CGA(T* model);

    /**
     * @brief Initialize GA structures and allocate memory
     *
     * Sets up:
     * - Population arrays
     * - Fitness distribution
     * - Model copies for parallel evaluation
     * - Parameter ranges
     *
     * @pre GA_params must be configured
     * @pre Parameter ranges (minval, maxval) must be set
     * @post Ready for InitiatePopulation() call
     */
    void initialize();

    /**
     * @brief Read best solution from previous output file
     * @param filename Path to GA output file
     * @return Best fitness value found in file
     *
     * Extracts best individual from previous run for warm-starting
     * or comparison.
     */
    double getfromoutput(std::string filename);

    /**
     * @brief Initialize population from previous output file
     * @param filename Path to output file containing population data
     *
     * Reads population from GA output file format. Extracts parameter
     * values from previous run's final generation to use as starting population.
     */
    void getinifromoutput(std::string filename);

    /**
     * @brief Read initial population from file
     * @param filename Path to population file
     *
     * Loads pre-specified starting population from file.
     * Used when GA_params.readfromgafile is true.
     */
    void getinitialpop(std::string filename);

    /**
     * @brief Main optimization loop
     * @return 0 on success, negative on error
     *
     * Executes complete GA workflow:
     * 1. Initialize/load population
     * 2. Evaluate fitness
     * 3. For each generation:
     *    a. Selection
     *    b. Crossover
     *    c. Mutation
     *    d. Fitness evaluation
     *    e. Statistics and output
     * 4. Return best solution
     *
     * @pre Population must be initialized via InitiatePopulation()
     * @post final_params contains best solution
     * @post Results written to output file
     *
     * @see InitiatePopulation()
     */
    int optimize();

    /**
     * @brief Set GA property from string key-value pair
     * @param varname Property name (e.g., "population_size", "mutation_rate")
     * @param value Property value as string
     * @return true if property was recognized and set successfully
     *
     * Allows configuration from text files or user input.
     * Supports all fields in GA_params structure.
     */
    bool SetProperty(const std::string &varname, const std::string &value);

    /**
     * @brief Set multiple GA properties from map
     * @param arguments Map of property names to values
     * @return true if all properties were set successfully
     *
     * Batch property setting for configuration management.
     */
    bool SetProperties(const std::map<std::string,std::string> &arguments);

    /**
     * @brief Create initial population
     *
     * Generates starting population using one of:
     * - Random initialization within parameter bounds (default)
     * - Loading from file (if readfromgafile is true)
     * - Custom initial population (if initial_pop is set)
     *
     * @pre GA must be initialized via initialize()
     * @pre Parameter bounds must be set
     * @post Ind vector contains maxpop individuals
     * @post All individuals have random or loaded parameter values
     *
     * @see initialize()
     */
    void InitiatePopulation();

    /**
     * @brief Last error message from GA execution
     *
     * If GA fails or encounters issues, error description stored here.
     */
    std::string last_error;

#ifdef Q_GUI_SUPPORT
    /**
     * @brief Set progress window for GUI updates
     * @param _rtw Pointer to ProgressWindow object
     *
     * Links GA to progress dialog for real-time status updates including:
     * - Current generation number
     * - Best fitness value
     * - Population statistics
     * - Convergence plots
     */
    void SetRunTimeWindow(ProgressWindow *_rtw) {
        rtw = _rtw;
    }
#endif

private:
    /**
     * @brief Set parameter bounds and precision
     * @param a Parameter index
     * @param minrange Lower bound
     * @param maxrange Upper bound
     * @param prec Precision/discretization level (for binary encoding)
     *
     * Configures search space boundaries for individual parameter.
     */
    void Setminmax(int a, double minrange, double maxrange, int prec);

    /**
     * @brief Initialize fitness distribution for selection
     *
     * Sets up cumulative fitness distribution (CDF) used in
     * roulette wheel selection for choosing parents.
     */
    void fitnessdistini();

    /**
     * @brief Perform crossover operation on population
     *
     * Applies crossover operator to selected parent pairs to generate
     * offspring. Type of crossover determined by GA_params.cross_over_type.
     *
     * @post Offspring replace parents (or are added to population)
     */
    void crossover();

    /**
     * @brief Calculate average fitness of population
     * @return Mean fitness value across all individuals
     *
     * Used for monitoring convergence and population quality.
     */
    double avgfitness();

    /**
     * @brief Apply mutation to population
     * @param mu Mutation probability per gene
     *
     * Randomly perturbs parameters of individuals with probability mu.
     * Maintains genetic diversity and enables exploration.
     *
     * @param mu Probability of mutating each parameter (0-1)
     */
    void mutate(double mu);

    /**
     * @brief Evaluate fitness for all individuals in population
     *
     * Calculates objective function value for each individual.
     * Can be parallelized across multiple threads.
     *
     * @post Each individual's fitness member is updated
     * @post sumfitness and MaxFitness are updated
     */
    void assignfitnesses();

    /**
     * @brief Find index of individual with maximum fitness
     * @return Index of best individual in Ind vector
     *
     * Identifies elite individual for preservation or output.
     */
    int maxfitness();

    /**
     * @brief Calculate variance of population fitness
     * @return Fitness variance across population
     *
     * Measure of population diversity. Low variance indicates convergence
     * (all individuals similar). High variance indicates diverse population.
     */
    double variancefitness();

    /**
     * @brief Calculate standard deviation of population fitness
     * @return Fitness standard deviation
     *
     * Square root of fitness variance. Easier to interpret as it's in
     * same units as fitness.
     */
    double stdfitness();

    /**
     * @brief Calculate average of actual (untransformed) fitness values
     * @return Mean of raw fitness values
     *
     * Average fitness before any scaling or transformation.
     */
    double avg_actual_fitness();

    /**
     * @brief Write detailed GA information to file
     * @param s String to append to detailed log
     *
     * Logs generation-by-generation statistics for analysis:
     * - Best/worst/average fitness
     * - Population diversity metrics
     * - Parameter value distributions
     */
    void write_to_detailed_GA(std::string s);

    /**
     * @brief Set population size
     * @param n Number of individuals
     *
     * Allocates memory for population of size n.
     *
     * @post GA_params.maxpop is updated
     * @post Ind and Ind_old vectors are resized
     */
    void setnumpop(int n);

    /**
     * @brief Calculate average of inverse actual fitness
     * @return Mean of 1/fitness values
     *
     * Used for certain fitness scaling schemes where lower raw values
     * are better (minimization).
     */
    double avg_inv_actual_fitness();

    /**
     * @brief Optimize for specified number of generations
     * @param nGens Number of generations to evolve
     * @param DefOutPutFileName Default output filename
     * @return 0 on success
     *
     * Alternative optimize interface with explicit generation count
     * and output file specification.
     */
    int optimize(int nGens, char DefOutPutFileName[]);

    /**
     * @brief Set number of parameters
     * @param n Parameter count
     *
     * Allocates storage for n-dimensional parameter vectors.
     *
     * @post GA_params.nParam is updated
     * @post Parameter-related vectors are resized
     */
    void setnparams(int n);

    /**
     * @brief Assign fixed values to non-optimized parameters
     *
     * For each individual, sets non-variable parameters to their
     * fixed values from fixedinputvale vector.
     */
    void assignfixedvalues();

    /**
     * @brief Assign ranks to individuals based on fitness
     *
     * Sorts population by fitness and assigns ranks (1 = best).
     * Used for rank-based selection methods.
     *
     * @post Individuals ordered by fitness
     */
    void assignrank();

    /**
     * @brief Assign fitness using rank-based scheme
     * @param N Selection pressure parameter
     *
     * Calculates fitness from rank using linear ranking formula:
     * fitness(rank) = N - 2(N-1)(rank-1)/(pop_size-1)
     *
     * Reduces selection pressure compared to raw fitness, preventing
     * premature convergence.
     *
     * @param N Selection pressure (typically 1.5-2.0)
     */
    void assignfitness_rank(double N);

    /**
     * @brief Apply shake/perturbation to population
     *
     * Randomly perturbs all individuals to escape local optima or
     * restore diversity. Perturbation magnitude controlled by
     * GA_params.shakescale.
     *
     * Useful when population has converged but not to optimum.
     */
    void shake();

    /**
     * @brief Real-coded crossover operation
     *
     * Performs crossover for real-valued (continuous) representation
     * using arithmetic or blend crossover operators.
     *
     * @note Used when GA_params.RCGA is true
     * @see crossover()
     */
    void crossoverRC();

    /**
     * @brief Read population from file
     * @param filename Path to population file
     *
     * Loads population state from checkpoint or previous run.
     */
    void getfromfile(char filename[]);

    /**
     * @brief Fill fitness distribution for roulette wheel selection
     *
     * Constructs cumulative distribution function (CDF) from individual
     * fitness values for probabilistic parent selection.
     *
     * @pre assignfitnesses() must have been called
     * @post fitdist is ready for sampling
     */
    void fillfitdist();

    /**
     * @brief Evaluate fitness for specific parameter vector
     * @param inp Parameter values to evaluate
     * @return Fitness value for given parameters
     *
     * Single-point fitness evaluation (not on entire population).
     * Used for local search refinement.
     */
    double assignfitnesses(std::vector<double> inp);

    /**
     * @brief Perform gradient descent local search
     * @param inp Starting parameter values
     * @return Improved fitness after local search
     *
     * Applies gradient-based refinement to individual for hybrid GA.
     * Accelerates convergence in neighborhood of optimum.
     *
     * @note Used when GA_params.Steepest_Descent is true
     */
    double Gradient_Descent(std::vector<double> inp);

    /**
     * @brief Get parameter index for variable and time series
     * @param i Variable index
     * @param ts Time series index
     * @return Parameter index in optimization vector
     */
    int getparamno(int i, int ts);

    /**
     * @brief Get active parameter number
     * @param i Parameter index
     * @return Active parameter index (excluding fixed parameters)
     */
    int get_act_paramno(int i);

    /**
     * @brief Get time series index for parameter
     * @param i Parameter index
     * @return Time series index
     */
    int get_time_series(int i);

    /**
     * @brief Evaluate model forward (compute predictions)
     * @return Objective function value (model-data misfit)
     *
     * Runs model with current parameters and calculates error relative
     * to observations.
     */
    double evaluateforward();

    /**
     * @brief Evaluate model with mixed parameters
     * @param v Parameter vector (subset or transformed)
     * @return Objective function value
     *
     * Variation of forward evaluation for special parameter handling.
     */
    double evaluateforward_mixed(std::vector<double> v);

    /**
     * @brief Current generation counter
     *
     * Tracks which generation is currently being evolved.
     * Used for output, termination, and adaptive operators.
     */
    int current_generation = 0;

    /**
     * @brief Fitness distribution for parent selection
     *
     * Cumulative distribution function used in roulette wheel selection.
     */
    GADistribution fitdist;

    /**
     * @brief Number of threads for parallel fitness evaluation
     *
     * Controls parallelization of fitness calculations across population.
     */
    int numberOfThreads;

#ifdef Q_GUI_SUPPORT
    /**
     * @brief Pointer to progress window for GUI updates
     *
     * If not nullptr, GA updates progress bar and displays current
     * generation, best fitness, convergence plot, etc.
     */
    ProgressWindow *rtw = nullptr;
#endif
};

#include "GA.hpp"

#endif // CGACLASS
