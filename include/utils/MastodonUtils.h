/*************************************************/
/*           DO NOT MODIFY THIS HEADER           */
/*                                               */
/*                     MASTODON                  */
/*                                               */
/*    (c) 2015 Battelle Energy Alliance, LLC     */
/*            ALL RIGHTS RESERVED                */
/*                                               */
/*   Prepared by Battelle Energy Alliance, LLC   */
/*     With the U. S. Department of Energy       */
/*                                               */
/*     See COPYRIGHT for full restrictions       */
/*************************************************/

#ifndef MASTODONUTILS_H
#define MASTODONUTILS_H

// MOOSE includes
#include "GeneralVectorPostprocessor.h"
// Other includes
#include <boost/math/distributions.hpp>

// Forward Declarations
namespace MastodonUtils
{
/**
 *  The responseSpectrum function calculates the response spectrum for a
 *  given acceleration history.
 */
std::vector<std::vector<Real>> responseSpectrum(const Real & freq_start,
                                                const Real & freq_end,
                                                const unsigned int & freq_num,
                                                const std::vector<Real> & history_acc,
                                                const Real & xi,
                                                const Real & reg_dt);

/**
 *  The regularize function inputs a vector or a vector of vectors (with the
 *  first vector being time) and outputs a vector with a constant dt. The
 *  input time vector has to be monotonically increasing.
 */
std::vector<std::vector<Real>> regularize(const std::vector<Real> & history_acc,
                                          const std::vector<Real> & history_time,
                                          const Real & reg_dt);

/**
 * Function to check if all the vectors in a vector are of equal size
 */
bool checkEqualSize(const std::vector<std::vector<Real>> & vectors);

/**
 * Function to check if two vectors are equal
 */
bool checkEqual(const std::vector<Real> & vector1,
                const std::vector<Real> & vector2,
                const Real percent_error = 0.0);

/**
 * Function to check if the vector contains any non-positive elements
 */
bool isNegativeOrZero(const std::vector<Real> & vector);

/**
 * Function to calculate the mean of the elements in a vector
 */
Real mean(const std::vector<Real> & vector);

/**
 * Function to calculate the median of the elements in a vector
 */
Real median(const std::vector<Real> & vector, const std::string & interpolation = "linear");

/**
 * Function to calculate the percentile value of the elements in a vector
 */
Real percentile(const std::vector<Real> & vector,
                const Real & percent,
                const std::string & interpolation = "linear");

/**
 * Function to calculate the standard deviation of the elements in a vector
 */
Real standardDeviation(const std::vector<Real> & vector);

/**
 * Function to calculate the lognormal standard deviation of the elements in a vector
 */
Real lognormalStandardDeviation(const std::vector<Real> & vector);

/**
 * Function template to calculate the probability that one random variable is greater
 * than another random variable
 */
template <typename T>
Real
greaterProbability(T & demand_distribution, T & capacity_distribution)
{
  Real min_demand =
      boost::math::quantile(demand_distribution, 0.001); //~ -3 sigma for normal distributions
  Real max_demand =
      boost::math::quantile(demand_distribution, 0.999); //~ +3 sigma for normal distributions
  Real prob = 0.0;
  Real param = min_demand;
  Real p_1, p_2;
  Real delta = boost::math::median(demand_distribution) / 1000;
  while (param < max_demand)
  {
    p_1 = boost::math::pdf(demand_distribution, param) *
          boost::math::cdf(capacity_distribution, param);
    p_2 = boost::math::pdf(demand_distribution, param + delta) *
          boost::math::cdf(capacity_distribution, param + delta);
    prob += delta * (p_1 + p_2) / 2;
    param += delta;
  }
  return prob;
}

/**
 * This function zeropads the number n with zeros in the beginning and makes n
 * the same number of digits as n_tot. The function returns the zeropadded string.
 */
std::string zeropad(const unsigned int n, const unsigned int n_tot);

} // namespace MastodonUtils
#endif
