// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#pragma once

#include <vector>
#include <string>
#include <optional>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include "armadillo"

#ifdef Q_JSON_SUPPORT
#include <QList>
#include <QJsonObject>
#include <QMap>
#include <QVariant>
#endif

#ifdef TORCH_SUPPORT
#include <torch/torch.h>
#endif

 /**
  * @brief Represents a data point in the time series.
  */
template<typename T>
struct DataPoint {
    T t{};                       ///< Time
    T c{};                       ///< Value
    std::optional<T> d;          ///< Duration or distance (optional)
};

/**
 * @brief Generic time series class template for storing and manipulating time-value pairs.
 */
template<typename T>
class TimeSeries : public std::vector<DataPoint<T>> {
public:
    using Base = std::vector<DataPoint<T>>;

    // -------------------------------------------------------------------------
    // Constructors and Assignment
    // -------------------------------------------------------------------------

    TimeSeries();                                          ///< Default constructor
    explicit TimeSeries(size_t n);                         ///< Create with n default-initialized points (explicit: a number must not silently become a series)
    TimeSeries(const Base& points);                        ///< Construct from vector of DataPoints
    TimeSeries(const std::vector<T>& t, const std::vector<T>& c); ///< From separate time and value vectors
    TimeSeries(const TimeSeries<T>& other);                ///< Copy constructor
    TimeSeries(TimeSeries<T>&& other) noexcept;            ///< Move constructor
    TimeSeries(const std::vector<T>& data, int interval);  ///< From raw vector and sampling interval
    explicit TimeSeries(const std::string& filename);      ///< Construct by reading from file
    TimeSeries(T a, T b, const std::vector<T>& x);         ///< From linear transformation of x
    TimeSeries(T a, T b, const TimeSeries<T>& btc);        ///< From linear transformation of another TimeSeries

#ifdef _arma
    TimeSeries(const arma::mat& x, const arma::mat& y);    ///< From Armadillo matrices
#endif

    TimeSeries<T>& operator=(const TimeSeries<T>& other);  ///< Copy assignment
    TimeSeries<T>& operator=(TimeSeries<T>&& other) noexcept; ///< Move assignment

    // -------------------------------------------------------------------------
    // Destruction
    // -------------------------------------------------------------------------

    ~TimeSeries();

    // -------------------------------------------------------------------------
    // Metadata (Name, Unit, Filename)
    // -------------------------------------------------------------------------

    const std::string& name() const;                       ///< Get name of the series
    const std::string& unit() const;                       ///< Get unit of the values
    std::string getFilename() const;                       ///< Get associated filename

    void setName(const std::string& name);                 ///< Set series name
    void setUnit(const std::string& unit);                 ///< Set unit label
    void setFilename(const std::string& filename);         ///< Set associated filename

    // -------------------------------------------------------------------------
    // Basic Accessors
    // -------------------------------------------------------------------------

    T getTime(size_t i) const;                             ///< Get time at index i
    T getValue(size_t i) const;                            ///< Get value at index i
    std::optional<T> getDistance(size_t i) const;          ///< Get duration at index i (if set)
    T& lastD();                                            ///< Get last duration by reference
    T& lastC();                                            ///< Get last value by reference
    T& lastt();                                            ///< Get last time by reference

    // -------------------------------------------------------------------------
    // Modification & Management
    // -------------------------------------------------------------------------

    void clear();                                          ///< Clear all data
    bool resize(unsigned int newSize);                     ///< Resize internal data vectors
    //void adjust_size(size_t new_size);                     ///< Adjust size to new_size exactly
    void setNumPoints(size_t n);                           ///< Resize to n points (alias)
	void removeNaNs();                                     ///< Remove points with NaN values
	TimeSeries<T> removeNaNs() const; 				       ///< Return new TimeSeries without NaNs
    void addPoint(T t, T c, std::optional<T> d = std::nullopt); ///< Add new DataPoint
    bool append(T value);                                  ///< Append value at t = 0
    bool append(T t, T c);                                 ///< Append time-value pair
	void append(const TimeSeries<T>& other);               ///< Append another TimeSeries to the end
    void assign_D();                                       ///< Assign durations (d) based on value change intervals

    // -------------------------------------------------------------------------
    // Structural Properties
    // -------------------------------------------------------------------------

    bool isStructured() const {
        return structured_;
    };                                                     ///< Check if series is structured
	void setStructured(bool structured) {
		structured_ = structured;
	}                                                       ///< Set structured status
    void detectStructure();                                ///< Identify if series is regularly spaced
    int Capacity() const;                                  ///< Get internal capacity


    std::vector<T> tToStdVector() const;                    /// < Times to stt::vector

    std::vector<double> ValuesToStdVector() const;           /// < Values to std::vector
    // -------------------------------------------------------------------------
    // File I/O
    // -------------------------------------------------------------------------

    void writefile(const std::string& filename) const;     ///< Write to file
    bool readfile(const std::string& filename);            ///< Read from file

    // -------------------------------------------------------------------------
    // Interpolation
    // -------------------------------------------------------------------------

    T interpol(const T& x) const;                          ///< Interpolate at x
    T interpol_D(const T& x) const;                        ///< Interpolate duration at x
    T interpol(const T& x, const TimeSeries<T> &CumulativeDistribution, const double &correlationlength, bool addpoint = false );  ///<Interpolation with Gaussian sequential noise with exponential correlation function
    TimeSeries<T> interpol(const std::vector<T>& x) const; ///< Interpolate over vector
    TimeSeries<T> interpol(const TimeSeries<T>& x) const;  ///< Interpolate over TimeSeries
    TimeSeries<T> interpol(const TimeSeries<T>* x) const;  ///< Interpolate over pointer

    // -------------------------------------------------------------------------
    // Statistics
    // -------------------------------------------------------------------------

    T mean() const;                                        ///< Mean of values
    T mean(int start_item) const;                          ///< Mean from index
    T sum(int start_item=0) const;                         ///< Sum from index
    T stddev(int start_item=0) const;                      ///< Stddev from index
    T mean_log(int start_index=0) const;                   ///< Log-mean from index
    T average() const;                                     ///< Average via integration
    T average(T time) const;                               ///< Average up to time
    T slope() const;                                       ///< Slope between last two points
    T variance() const;                                    ///< Variance via integral formula
    T percentile(T p) const;                               ///< Percentile value
    T percentile(T p, int start_index) const;              ///< Percentile from index

    T maxC() const;                                        ///< Max value
    T minC() const;                                        ///< Min value
    T maxVal() const;                                      ///< Max alias
    T minVal() const;                                      ///< Min alias
    T maxAbsVal() const;                                   ///< Max abs
    T minAbsVal();                                         ///< Min abs

    T maxt() const;                                        ///< Max time
    T mint() const;                                        ///< Min time
    T maxfabs() const;                                     ///< Cached max abs (with tracking)
    T computeMaxFabs() const;                              ///< Recompute max abs
    void setMaxFabs(T val);                                ///< Manually set cached max abs
    T getMaxFabs() const;                                  ///< Access cached max abs

    // -------------------------------------------------------------------------
    // Transformations
    // -------------------------------------------------------------------------

    TimeSeries<T> derivative() const;                      ///< Compute dC/dt
    TimeSeries<T> log() const;                             ///< Natural log
    TimeSeries<T> log(T minval) const;                     ///< Log with min threshold
    TimeSeries<T> Exp() const;                             ///< Elementwise exp
    TimeSeries<T> fabs() const;                            ///< Elementwise abs
    TimeSeries<T> movingAverageSmooth(int span) const;     ///< Moving average smoothing
    TimeSeries<T> ConvertToRanks() const;                  ///< Map values to empirical ranks
    T Score(const T& val) const;                           ///< Rank score for value
    TimeSeries<T> inverse() const;                         ///< Swaps t and val
    TimeSeries<T> inverse_cummulative_uniform(int nbins = 100) const;                         ///< Calculates the inverse cummulative uniformaized on the x axis
    T inverse_CDF(T x) const;                              ///< Extract the value corresponding to the CDF
#ifdef GSL

    TimeSeries<T> ConvertToNormalScore() const;            ///< Gaussianize using inverse CDF
#endif

    // -------------------------------------------------------------------------
    // Kernel Operators
    // -------------------------------------------------------------------------

    T Exponential_Kernel(const T& t, const T& lambda) const; ///< Apply exponential kernel
    T Gaussian_Kernel(const T& t, const T& mu, const T& stdev) const; ///< Apply Gaussian kernel

    // -------------------------------------------------------------------------
    // Derived / Extracted Series
    // -------------------------------------------------------------------------

    TimeSeries<T> extract(T t1, T t2) const;               ///< Extract window
    TimeSeries<T> make_uniform(T increment, bool assignD = false) const; ///< Make uniform
    TimeSeries<T> getcummulative() const;                 ///< Cumulative integral
    TimeSeries<T> GetCummulativeDistribution() const;     ///< Sorted value CDF
    TimeSeries<T> distribution(int n_bins, int start_index) const; ///< Histogram density

    // Append operators
    TimeSeries<T>& operator+=(const TimeSeries<T>& v);   // Add with interpolation
    TimeSeries<T>& operator%=(const TimeSeries<T>& v);   // Add pointwise (same timestamps)

    // Integration
    T integrate() const;
    T integrate(T upper_time) const;
    T integrate(T t1, T t2) const;

    // Value/time setters
    bool setTime(size_t i, const T& value);
    bool setValue(size_t i, const T& value);
    bool setDistance(size_t i, const T& value);

    // Value/time getters
    T getLastValue() const;
    T getLastTime() const;

    // -------------------------------------------------------------------------
    // Time Lookup & Modification
    // -------------------------------------------------------------------------

    int lookupt(T time) const;                            ///< Index of t[i] <= time < t[i+1]
    int GetElementNumberAt(const T& x) const;             ///< Find index enclosing time x
    void knock_out(T cutoff_time);                        ///< Delete all points after cutoff

    // -------------------------------------------------------------------------
    // Time-Based Analysis
    // -------------------------------------------------------------------------

    T mean_t() const;                                     ///< Mean of time stamps
    std::vector<T> trend() const;                         ///< Linear regression [intercept, slope]
    T wiggle() const;                                     ///< Oscillation metric
    T wiggle_corr(int n_back) const;                      ///< Correlation-like metric
    bool wiggle_sl(T tol) const;                          ///< Detect alternating slope pattern

    T AutoCor1(int k) const;                              ///< Lag-1 autocorrelation
    T AutoCorrelation(const T& distance) const;           ///< At lag
    TimeSeries<T> AutoCorrelation(const T& span, const T& increment) const; ///< Curve
    double AutoCorrelationCoeff() const;                  ///< Coarse slope vs log(|value|)

    // -------------------------------------------------------------------------
    // Noise / Stochastic
    // -------------------------------------------------------------------------

    TimeSeries<T> add_noise(T stddev, bool logd = false) const; ///< Add Gaussian noise

#ifdef GSL
    void CreateOUProcess(const T& t_start, const T& t_end, const T& dt, const T& theta); ///< Ornstein-Uhlenbeck
    TimeSeries<T> MapfromNormalScoreToDistribution(const std::string& distribution, const std::vector<double>& parameters);
    TimeSeries<T> MapfromNormalScoreToDistribution(const TimeSeries<double>& distribution);
#endif

    void CreatePeriodicStepFunction(const T& t_start, const T& t_end, const T& duration, const T& gap, const T& magnitude);
    TimeSeries<T> LogTransformX() const;

    // -------------------------------------------------------------------------
    // Serialization
    // -------------------------------------------------------------------------

#ifdef Q_JSON_SUPPORT
    QJsonObject toJson() const;                           ///< Convert to QJsonObject
    void fromJson(const QJsonObject& obj);                ///< Load from QJsonObject
#endif // Q_JSON_SUPPORT

    bool fileNotFound = false;
    bool fileNotCorrect = false;
    void setdt(double dt) {dt_=dt;}

#ifdef TORCH_SUPPORT
    /**
     * @brief Convert TimeSeries values to a torch::Tensor for ANN training.
     * @param include_time If true, returns a 2D tensor with [time, value] pairs.
     *                     If false, returns a 1D tensor with only values.
     * @param device Target device for the tensor (CPU, CUDA, etc.)
     * @return torch::Tensor containing the time series data
     */
    torch::Tensor toTensor(bool include_time = false,
                           torch::Device device = torch::kCPU) const;

    /**
     * @brief Convert TimeSeries to a normalized torch::Tensor.
     * @param include_time If true, includes normalized time values.
     * @param value_min Minimum value for normalization (uses series min if not provided)
     * @param value_max Maximum value for normalization (uses series max if not provided)
     * @param device Target device for the tensor
     * @return torch::Tensor with values normalized to [0, 1] range
     */
    torch::Tensor toNormalizedTensor(bool include_time = false,
                                     std::optional<T> value_min = std::nullopt,
                                     std::optional<T> value_max = std::nullopt,
                                     torch::Device device = torch::kCPU) const;

    /**
     * @brief Convert TimeSeries to a sliding window tensor for sequence modeling.
     * @param window_size Size of each window
     * @param stride Step size between windows
     * @param include_time Whether to include time values in each window
     * @param device Target device for the tensor
     * @return torch::Tensor of shape [num_windows, window_size, features]
     */
    torch::Tensor toSlidingWindowTensor(int window_size,
                                        int stride = 1,
                                        bool include_time = false,
                                        torch::Device device = torch::kCPU) const;

    /**
 * @brief Create a TimeSeries from a torch::Tensor.
 * @param tensor Input tensor (1D for values only, 2D for [time, value] pairs)
 * @param has_time If true, treats 2D tensor as [time, value] pairs
 * @param time_offset Starting time value if tensor contains values only
 * @param time_step Time increment if tensor contains values only
 * @param start_time Optional start time - if not provided, uses entire tensor
 * @param end_time Optional end time - if not provided, uses entire tensor
 * @return TimeSeries constructed from tensor data
 */
    static TimeSeries<T> fromTensor(const torch::Tensor& tensor,
                                    bool has_time = false,
                                    T time_offset = T{0},
                                    T time_step = T{1},
                                    std::optional<T> start_time = std::nullopt,
                                    std::optional<T> end_time = std::nullopt);
    /**
     * @brief Generate tensor with interpolated values at specified time intervals.
     * @param t_start Start time
     * @param t_end End time
     * @param dt Time step increment
     * @param include_time If true, returns [time, value] pairs
     * @param device Target device for the tensor
     * @return torch::Tensor with interpolated values at uniform time intervals
     */
    torch::Tensor toTensorAtIntervals(T t_start, T t_end, T dt,
                                      bool include_time = false,
                                      torch::Device device = torch::kCPU) const;

    /**
     * @brief Generate tensor with interpolated values at specified time points.
     * @param time_points Vector of time points where values should be interpolated
     * @param include_time If true, returns [time, value] pairs
     * @param device Target device for the tensor
     * @return torch::Tensor with interpolated values at specified times
     */
    torch::Tensor toTensorAtTimes(const std::vector<T>& time_points,
                                  bool include_time = false,
                                  torch::Device device = torch::kCPU) const;

    /**
     * @brief Generate normalized tensor with interpolated values at time intervals.
     * @param t_start Start time
     * @param t_end End time
     * @param dt Time step increment
     * @param include_time If true, includes normalized time values
     * @param value_min Minimum value for normalization (auto-calculated if not provided)
     * @param value_max Maximum value for normalization (auto-calculated if not provided)
     * @param device Target device for the tensor
     * @return torch::Tensor with normalized interpolated values
     */
    torch::Tensor toNormalizedTensorAtIntervals(T t_start, T t_end, T dt,
                                                bool include_time = false,
                                                std::optional<T> value_min = std::nullopt,
                                                std::optional<T> value_max = std::nullopt,
                                                torch::Device device = torch::kCPU) const;

    /**
     * @brief Generate sliding window tensor from interpolated uniform time grid.
     * @param t_start Start time
     * @param t_end End time
     * @param dt Time step increment
     * @param window_size Size of each window
     * @param stride Step size between windows
     * @param include_time Whether to include time values in each window
     * @param device Target device for the tensor
     * @return torch::Tensor of shape [num_windows, window_size, features]
     */
    torch::Tensor toSlidingWindowTensorAtIntervals(T t_start, T t_end, T dt,
                                                   int window_size, int stride = 1,
                                                   bool include_time = false,
                                                   torch::Device device = torch::kCPU) const;

#endif // TORCH_SUPPORT
private:
    bool structured_ = false;
    T dt_ = 0;
    T max_fabs_ = T{};
    bool max_fabs_valid_ = false;

    std::string name_;
    std::string unit_;
    std::string filename_;

#ifdef GSL
    const gsl_rng_type* A_ = nullptr;
    gsl_rng* r_ = nullptr;
    void ensureGSLInitialized();
#endif




};


/**
 * @brief Computes the squared L2 norm of a time series (sum of squared values).
 * @param series Input time series.
 * @return Norm squared.
 */
template<typename T>
T norm2(const TimeSeries<T>& series);

/**
 * @brief Computes an R²-style correlation using absolute values of signals.
 * @param series1 First time series.
 * @param series2 Second time series.
 * @return R²-style coefficient.
 */
template<typename T>
T R2_c(const TimeSeries<T>& series1, const TimeSeries<T>& series2);

/**
 * @brief Returns a copy of the time series with all values clamped below `scalar` raised to `scalar`.
 * @param series Input series.
 * @param scalar Threshold.
 * @return Clamped series.
 */
template<typename T>
TimeSeries<T> max(const TimeSeries<T>& series, T scalar);

// Scalar multiplication
template<typename T>
TimeSeries<T> operator*(T alpha, const TimeSeries<T>& ts);


template<typename T>
TimeSeries<T> operator*(const TimeSeries<T>& ts, T alpha);

// Scalar division
template<typename T>
TimeSeries<T> operator/(const TimeSeries<T>& ts, T alpha);

// Pointwise division by interpolated values
template<typename T>
TimeSeries<T> operator/(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2);

// Pointwise subtraction with interpolation
template<typename T>
TimeSeries<T> operator-(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2);

// Pointwise product with interpolation
template<typename T>
TimeSeries<T> operator*(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2);

// TimeSeries minus scalar
template<typename T>
TimeSeries<T> operator-(const TimeSeries<T>& ts, T scalar);

// TimeSeries divided by scalar (redundant, already declared but consistent)
template<typename T>
TimeSeries<T> operator/(const TimeSeries<T>& ts, T scalar);

// Pointwise division without interpolation (assumes same time stamps)
template<typename T>
TimeSeries<T> operator%(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2);

// Pointwise addition without interpolation (assumes same time stamps)
template<typename T>
TimeSeries<T> operator&(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2);

// --- Scalar Multiplication ---
/**
    * @brief Multiplies a scalar with a time series (scalar * series).
    */
template<typename T>
TimeSeries<T> operator*(T alpha, const TimeSeries<T>& ts);

/**
     * @brief Multiplies a time series with a scalar (series * scalar).
     */
template<typename T>
TimeSeries<T> operator*(const TimeSeries<T>& ts, T alpha);


// --- Scalar Division ---
/**
     * @brief Divides a time series by a scalar.
     */
template<typename T>
TimeSeries<T> operator/(const TimeSeries<T>& ts, T alpha);


// --- Pointwise Division (Interpolated) ---
/**
     * @brief Divides one time series by another using interpolation.
     */
template<typename T>
TimeSeries<T> operator/(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2);


// --- Pointwise Subtraction (Interpolated) ---
/**
     * @brief Subtracts one time series from another using interpolation.
     */
template<typename T>
TimeSeries<T> operator-(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2);


// --- Pointwise Multiplication (Interpolated) ---
/**
     * @brief Multiplies two time series using interpolation.
     */
template<typename T>
TimeSeries<T> operator*(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2);


// --- Subtract Scalar ---
/**
     * @brief Subtracts a scalar from all values in the time series.
     */
template<typename T>
TimeSeries<T> operator-(const TimeSeries<T>& ts, T scalar);


// --- Pointwise Division (Aligned) ---
/**
     * @brief Divides two time series pointwise, assuming aligned time stamps.
     */
template<typename T>
TimeSeries<T> operator%(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2);


// --- Pointwise Addition (Aligned) ---
/**
     * @brief Adds two time series pointwise, assuming aligned time stamps.
     */
template<typename T>
TimeSeries<T> operator&(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2);

/**
 * @brief Pointwise subtraction: time series 1 - interpolated time series 2.
 */

template<typename T>
TimeSeries<T> operator-(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2);

/**
 * @brief Pointwise product using interpolation: ts1 * interpol(ts2).
 */
template<typename T>
TimeSeries<T> operator*(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2);

/**
 * @brief Pointwise division using interpolation: ts1 / interpol(ts2).
 */
template<typename T>
TimeSeries<T> operator/(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2);

/**
 * @brief Subtracts a scalar from all values.
 */
template<typename T>
TimeSeries<T> operator-(const TimeSeries<T>& ts, T scalar);

/**
 * @brief Multiplies every value in the series by a scalar.
 */
template<typename T>
TimeSeries<T> operator*(T scalar, const TimeSeries<T>& ts);

/**
 * @brief Multiplies every value in the series by a scalar (right-hand).
 */
template<typename T>
TimeSeries<T> operator*(const TimeSeries<T>& ts, T scalar);

/**
 * @brief Divides every value in the series by a scalar.
 */
template<typename T>
TimeSeries<T> operator/(const TimeSeries<T>& ts, T scalar);

/**
 * @brief Element-wise division without interpolation.
 * Assumes aligned timestamps.
 */
template<typename T>
TimeSeries<T> operator%(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2);

/**
 * @brief Element-wise addition without interpolation.
 * Assumes aligned timestamps.
 */
template<typename T>
TimeSeries<T> operator&(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2);

/**
 * @brief Pointwise difference without interpolation (misused '>' for legacy reasons).
 */
template<typename T>
TimeSeries<T> operator>(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2);

/**
    * @brief Computes the sum of interpolated values from multiple time series at a given time.
    * @param series_list Vector of time series.
    * @param time Target time.
    * @return Sum of interpolated values.
    */

template<typename T>
T sum_interpolate(const std::vector<TimeSeries<T>>& series_list, T time);


namespace TimeSeriesMetrics {
    
    
    /**
     * @brief Computes the empirical percentile of a scalar value in a vector.
     * @param values Vector of values.
     * @param x Fraction (0 to 1).
     * @return Percentile value corresponding to x.
     */
    template<typename T>
    T percentile(const std::vector<T>& values, T x);

    /**
     * @brief Computes the percentiles at specified quantiles.
     * @param values Vector of values.
     * @param fractions Vector of quantile fractions.
     * @return Vector of values at the given percentiles.
     */
    template<typename T>
    std::vector<T> percentile(const std::vector<T>& values, const std::vector<T>& fractions);

} // namespace TimeSeriesMetrics

template<typename T>
T percentile(const std::vector<T>& values, T x);

template<typename T>
std::vector<T> percentile(const std::vector<T>& values, const std::vector<T>& fractions);

/**
 * @brief Computes the sum of interpolated values across multiple time series at a given time.
 *
 * @param series_list Vector of time series to evaluate.
 * @param time The time at which to interpolate values.
 * @return T The sum of all interpolated values at the specified time.
 */
template<typename T>
T sum_interpolate(const std::vector<TimeSeries<T>>& series_list, T time);

template<typename T>
T Covariance(const TimeSeries<T>& BTC_p, const TimeSeries<T>& BTC_d);

template <typename T>
T covariance_mapped(const TimeSeries<T>& ts1, const TimeSeries<T>& ts2, int start_item = 0, bool sample = false);

inline int sign(double x) {
    return (x > 0) - (x < 0);
}

#include "TimeSeries.hpp"
