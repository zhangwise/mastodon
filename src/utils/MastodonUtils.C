// MASTODON includes
#include "MastodonUtils.h"
#include "MooseUtils.h"
#include "math.h"
#include "BoostDistribution.h"

std::vector<std::vector<Real>>
MastodonUtils::responseSpectrum(const Real & freq_start,
                                const Real & freq_end,
                                const unsigned int & freq_num,
                                const std::vector<Real> & history_acc,
                                const Real & xi,
                                const Real & reg_dt)
{
  std::vector<Real> freq_vec, aspec_vec, vspec_vec, dspec_vec;
  Real logdf, om_n, om_d, dt2, dis1, vel1, acc1, dis2, vel2, acc2, pdmax, kd;
  for (std::size_t n = 0; n < freq_num; ++n)
  {
    // Building the frequency vector. Frequencies are distributed
    // uniformly in the log scale.
    logdf = (log10(freq_end) - log10(freq_start)) / (freq_num - 1);
    freq_vec.push_back(pow(10.0, log10(freq_start) + n * logdf));
    om_n = 2.0 * 3.141593 * freq_vec[n]; // om_n = 2*pi*f
    om_d = om_n * xi;
    dis1 = 0.0;
    vel1 = 0.0;
    dt2 = reg_dt * reg_dt;
    pdmax = 0.0;
    acc1 = -1.0 * history_acc[0] - 2.0 * om_d * vel1 - om_n * om_n * dis1;
    kd = 1.0 + om_d * reg_dt + dt2 * om_n * om_n / 4.0;
    for (std::size_t j = 0; j < history_acc.size(); ++j)
    {
      dis2 = ((1.0 + om_d * reg_dt) * dis1 + (reg_dt + 1.0 / 2.0 * om_d * dt2) * vel1 +
              dt2 / 4.0 * acc1 - dt2 / 4.0 * history_acc[j]) /
             kd;
      acc2 = 4.0 / dt2 * (dis2 - dis1) - 4.0 / reg_dt * vel1 - acc1;
      vel2 = vel1 + reg_dt / 2.0 * (acc1 + acc2);
      if (std::abs(dis2) > pdmax)
        pdmax = std::abs(dis2);
      dis1 = dis2;
      vel1 = vel2;
      acc1 = acc2;
    }
    dspec_vec.push_back(pdmax);
    vspec_vec.push_back(pdmax * om_n);
    aspec_vec.push_back(pdmax * om_n * om_n);
  }
  return {freq_vec, dspec_vec, vspec_vec, aspec_vec};
}

std::vector<std::vector<Real>>
MastodonUtils::regularize(const std::vector<Real> & history_acc,
                          const std::vector<Real> & history_time,
                          const Real & reg_dt)
{
  std::vector<Real> reg_acc;
  std::vector<Real> reg_tme;
  Real cur_tme = history_time[0];
  Real cur_acc;
  for (std::size_t i = 0; i < history_time.size() - 1; ++i)
  {
    while (cur_tme >= history_time[i] && cur_tme <= history_time[i + 1])
    {
      cur_acc = history_acc[i] +
                (cur_tme - history_time[i]) / (history_time[i + 1] - history_time[i]) *
                    (history_acc[i + 1] - history_acc[i]);
      reg_acc.push_back(cur_acc);
      reg_tme.push_back(cur_tme);
      cur_tme += reg_dt;
    }
  }
  return {reg_tme, reg_acc};
}

bool
MastodonUtils::checkEqualSize(const std::vector<std::vector<Real>> & vectors)
{
  for (const auto & v : vectors)
  {
    if (v.size() != vectors[0].size())
      return false;
  }
  return true;
}

bool
MastodonUtils::checkEqual(const std::vector<Real> & vector1,
                          const std::vector<Real> & vector2,
                          const Real percent_error)
{
  if (vector1.size() != vector2.size())
    return false;
  for (std::size_t i = 0; i < vector1.size(); ++i)
  {
    if (!MooseUtils::absoluteFuzzyEqual(
            vector1[i], vector2[i], std::abs(vector1[i] * percent_error / 100)))
      return false;
  }
  return true;
}

bool
MastodonUtils::isNegativeOrZero(const std::vector<Real> & vector)
{
  for (const auto & element : vector)
    if (element <= 0)
      return true;
  return false;
}

Real
MastodonUtils::mean(const std::vector<Real> & vector)
{
  Real sum = std::accumulate(vector.begin(), vector.end(), 0.0);
  return sum / vector.size();
}

Real
MastodonUtils::median(const std::vector<Real> & vector, const std::string & interpolation)
{
  std::vector<Real> sorted_vector = vector;
  std::sort(sorted_vector.begin(), sorted_vector.end());
  if (sorted_vector.size() % 2 != 0)
    return sorted_vector[(sorted_vector.size() - 1) / 2];
  else if (interpolation == "linear")
    return (sorted_vector[sorted_vector.size() / 2] + sorted_vector[sorted_vector.size() / 2 - 1]) /
           2.0;
  else if (interpolation == "lower")
    return sorted_vector[sorted_vector.size() / 2 - 1];
  else if (interpolation == "higher")
    return sorted_vector[sorted_vector.size() / 2];
  else
    mooseError("Invalid interpolation type in median calculation.");
}

Real
MastodonUtils::percentile(const std::vector<Real> & vector,
                          const Real & percent,
                          const std::string & interpolation)
{
  std::vector<Real> sorted_vector = vector;
  std::sort(sorted_vector.begin(), sorted_vector.end());
  if (percent < 0.0 || percent > 100.0)
    mooseError("Percent should be between 0 and 100.\n");
  std::size_t low_index;
  if (floor(percent / 100 * sorted_vector.size()) == 0)
    low_index = 0;
  else
    low_index = floor(percent / 100 * sorted_vector.size()) - 1;
  if (interpolation == "lower")
    return sorted_vector[low_index];
  else if (interpolation == "higher")
    return sorted_vector[low_index + 1];
  else if (interpolation == "linear")
  {
    Real index_remainder = fmod(percent / 100.0 * sorted_vector.size(), 1.0);
    Real percentile_value =
        sorted_vector[low_index] +
        index_remainder * (sorted_vector[low_index + 1] - sorted_vector[low_index]);
    return percentile_value;
  }
  else
    mooseError("Invalid interpolation type in percentile calculation.");
}

Real
MastodonUtils::standardDeviation(const std::vector<Real> & vector)
{
  Real sum = 0;
  for (std::size_t i = 0; i < vector.size(); i++)
  {
    sum += (vector[i] - MastodonUtils::mean(vector)) * (vector[i] - MastodonUtils::mean(vector));
  }
  Real sigma = sqrt(sum / (vector.size() - 1));
  return sigma;
}

Real
MastodonUtils::lognormalStandardDeviation(const std::vector<Real> & vector)
{
  if (MastodonUtils::isNegativeOrZero(vector))
    mooseError("One or more elements in the sample for calculating beta are non positive.\n");
  std::vector<Real> logvector;
  for (const auto & element : vector)
    logvector.push_back(log(element));
  return MastodonUtils::standardDeviation(logvector);
}

std::string
MastodonUtils::zeropad(const unsigned int n, const unsigned int n_tot)
{
  std::size_t zeropadsize = (std::to_string(n_tot)).length() - (std::to_string(n)).length();
  std::string pad = "";
  for (std::size_t i = 0; i < zeropadsize; i++)
  {
    pad += "0";
  }
  return pad + std::to_string(n);
}
