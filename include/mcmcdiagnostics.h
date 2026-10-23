// SedSAT3 – Sediment Source Assessment Tool, version 3
// U.S. Geological Survey. See LICENSE.md and DISCLAIMER.md in the repository root.

#ifndef MCMCDIAGNOSTICS_H
#define MCMCDIAGNOSTICS_H

/**
 * @file mcmcdiagnostics.h
 * @brief Convergence diagnostics for MCMC chains: split R-hat and the
 * effective sample size
 *
 * Both follow Vehtari, Gelman, Simpson, Carpenter and Bürkner (2021),
 * "Rank-normalization, folding, and localization: an improved R-hat for
 * assessing convergence of MCMC", Bayesian Analysis 16(2), without the rank
 * normalization. Each chain is split in two halves, so a chain whose first
 * and second halves differ (a trend) also shows as non-converged.
 */

#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>
#include <gsl/gsl_fft_complex.h>

namespace mcmcdiagnostics
{

struct Diagnostics
{
    double rhat = std::numeric_limits<double>::quiet_NaN();              ///< split R-hat
    double effective_sample_size = std::numeric_limits<double>::quiet_NaN(); ///< multi-chain ESS
};

/// Autocovariance of x at lags 0..n-1 (divided by n), computed with an FFT
inline std::vector<double> Autocovariance(const std::vector<double>& x)
{
    const size_t n = x.size();
    std::vector<double> result(n, 0.0);
    if (n == 0)
        return result;
    double mean = 0;
    for (double v : x)
        mean += v;
    mean /= double(n);

    size_t length = 1;
    while (length < 2 * n)
        length <<= 1;
    std::vector<double> data(2 * length, 0.0);   // interleaved complex values
    for (size_t i = 0; i < n; i++)
        data[2 * i] = x[i] - mean;
    gsl_fft_complex_radix2_forward(data.data(), 1, length);
    for (size_t i = 0; i < length; i++)
    {
        const double re = data[2 * i], im = data[2 * i + 1];
        data[2 * i] = re * re + im * im;
        data[2 * i + 1] = 0;
    }
    gsl_fft_complex_radix2_inverse(data.data(), 1, length);
    for (size_t t = 0; t < n; t++)
        result[t] = data[2 * t] / double(n);
    return result;
}

/**
 * @brief Split R-hat and effective sample size of one quantity
 * @param chains The draws of each chain (after burn-in), in order
 *
 * Returns NaN values when there are fewer than two chains or fewer than
 * four draws per chain, or when the quantity does not vary.
 */
inline Diagnostics Diagnose(const std::vector<std::vector<double>>& chains)
{
    Diagnostics out;

    // Split every chain into two halves of equal length
    size_t length = std::numeric_limits<size_t>::max();
    for (const auto& chain : chains)
        length = std::min(length, chain.size());
    if (chains.size() < 2 || length < 4 || length == std::numeric_limits<size_t>::max())
        return out;
    const size_t half = length / 2;
    std::vector<std::vector<double>> split;
    for (const auto& chain : chains)
    {
        split.emplace_back(chain.begin(), chain.begin() + half);
        split.emplace_back(chain.begin() + (length - half), chain.begin() + length);
    }
    const size_t m = split.size();
    const double n = double(half);

    // Between- and within-chain variances
    std::vector<double> means(m, 0.0), variances(m, 0.0);
    for (size_t c = 0; c < m; c++)
    {
        for (double v : split[c])
            means[c] += v;
        means[c] /= n;
        for (double v : split[c])
            variances[c] += (v - means[c]) * (v - means[c]);
        variances[c] /= (n - 1);
    }
    double grand_mean = 0;
    for (double v : means)
        grand_mean += v;
    grand_mean /= double(m);
    double B = 0;
    for (double v : means)
        B += (v - grand_mean) * (v - grand_mean);
    B *= n / double(m - 1);
    double W = 0;
    for (double v : variances)
        W += v;
    W /= double(m);
    if (!(W > 0))
        return out;
    const double var_plus = (n - 1) / n * W + B / n;
    out.rhat = std::sqrt(var_plus / W);

    // Effective sample size from the autocorrelations of all chains,
    // truncated with Geyer's initial monotone sequence
    std::vector<double> mean_autocovariance(half, 0.0);
    for (const auto& chain : split)
    {
        const std::vector<double> acov = Autocovariance(chain);
        for (size_t t = 0; t < half; t++)
            mean_autocovariance[t] += acov[t] / double(m);
    }
    auto rho = [&](size_t t) {
        return 1.0 - (W - mean_autocovariance[t] * n / (n - 1)) / var_plus;
    };
    double tau = -1.0;
    double previous_pair = std::numeric_limits<double>::infinity();
    for (size_t t = 0; t + 1 < half; t += 2)
    {
        double pair = rho(t) + rho(t + 1);
        if (pair < 0)
            break;
        pair = std::min(pair, previous_pair);   // monotone
        tau += 2.0 * pair;
        previous_pair = pair;
    }
    const double total = double(m) * n;
    tau = std::max(tau, 1.0 / std::log10(total));
    out.effective_sample_size = total / tau;
    return out;
}

} // namespace mcmcdiagnostics

#endif // MCMCDIAGNOSTICS_H
