#include "../../include/filters/exponentialfilter.h"
#include "../../include/core/Constants.h"

/*
 * y[n] = alpha * x[n] + (1 - alpha) * y[n-1].
 */

ExponentialFilter::ExponentialFilter(double alpha)
    : FilterBase("Exponential"),
      m_alpha(Constants::Filters::DEFAULT_EXPONENTIAL_ALPHA),
      m_prevOutput(0.0f) {
  if (alpha < Constants::Filters::MIN_EXPONENTIAL_ALPHA) {
    m_alpha = Constants::Filters::MIN_EXPONENTIAL_ALPHA;
  } else if (alpha > Constants::Filters::MAX_EXPONENTIAL_ALPHA) {
    m_alpha = Constants::Filters::MAX_EXPONENTIAL_ALPHA;
  } else {
    m_alpha = alpha;
  }
}

DataPoint ExponentialFilter::filter(const DataPoint &input) {
  float out = static_cast<float>(m_alpha * input.value +
                                 (1.0 - m_alpha) * m_prevOutput);
  m_prevOutput = out;
  return DataPoint(input.timestamp, out);
}

void ExponentialFilter::reset() { m_prevOutput = 0.0f; }

bool ExponentialFilter::isReady() const { return true; }

void ExponentialFilter::setAlpha(double alpha) {
  if (alpha < Constants::Filters::MIN_EXPONENTIAL_ALPHA) {
    m_alpha = Constants::Filters::MIN_EXPONENTIAL_ALPHA;
  } else if (alpha > Constants::Filters::MAX_EXPONENTIAL_ALPHA) {
    m_alpha = Constants::Filters::MAX_EXPONENTIAL_ALPHA;
  } else {
    m_alpha = alpha;
  }
  m_prevOutput = 0.0f;
}

double ExponentialFilter::getAlpha() const { return m_alpha; }

size_t ExponentialFilter::getMemoryUsage() const {
  return sizeof(m_alpha) + sizeof(m_prevOutput);
}
