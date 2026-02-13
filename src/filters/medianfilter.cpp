#include "../../include/filters/medianfilter.h"
#include "../../include/core/Constants.h"
#include <algorithm>
#include <vector>

/*
 * медианный фильтр. в каждом шаге окна из последних N значений (N нечётное).
 */

MedianFilter::MedianFilter(size_t windowSize)
    : FilterBase("Median"), m_windowSize(5), m_window() {
  if (windowSize < Constants::Filters::MIN_MEDIAN_WINDOW) {
    m_windowSize =
        Constants::Filters::MIN_MEDIAN_WINDOW; // если мало, то минимальное
  } else if (windowSize > Constants::Filters::MAX_MEDIAN_WINDOW) {
    m_windowSize =
        Constants::Filters::MAX_MEDIAN_WINDOW; // если много, то максимальное
  } else {
    m_windowSize = windowSize; // если в пределах, то используем
  }
  if (m_windowSize % 2 == 0) { // если четное, то уменьшаем на 1
    m_windowSize--;            // чтобы было нечетное
  }
}

DataPoint MedianFilter::filter(const DataPoint &input) {
  m_window.push_back(input.value);

  while (m_window.size() > m_windowSize) {
    m_window.pop_front();
  }

  float median = computeMedian();
  return DataPoint(input.timestamp, median);
}

void MedianFilter::reset() { m_window.clear(); }

bool MedianFilter::isReady() const { return m_window.size() >= m_windowSize; }

void MedianFilter::setWindowSize(size_t windowSize) {
  if (windowSize < Constants::Filters::MIN_MEDIAN_WINDOW) {
    m_windowSize = Constants::Filters::MIN_MEDIAN_WINDOW;
  } else if (windowSize > Constants::Filters::MAX_MEDIAN_WINDOW) {
    m_windowSize = Constants::Filters::MAX_MEDIAN_WINDOW;
  } else {
    m_windowSize = windowSize;
  }
  if (m_windowSize % 2 == 0) {
    m_windowSize--;
  }
  m_window.clear();
}

size_t MedianFilter::getWindowSize() const { return m_windowSize; }

size_t MedianFilter::getMemoryUsage() const {
  return m_window.size() * sizeof(float) + sizeof(m_windowSize) +
         sizeof(m_window);
}

float MedianFilter::computeMedian() const {
  if (m_window.empty()) {
    return 0.0f;
  }

  std::vector<float> copy(m_window.begin(), m_window.end());
  size_t mid = (copy.size() - 1) / 2;

  std::nth_element(copy.begin(),
                   copy.begin() + static_cast<std::ptrdiff_t>(mid), copy.end());
  return copy[mid];
}
