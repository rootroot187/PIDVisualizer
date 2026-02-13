#include "../../include/filters/movingaveragefilter.h"
#include "../../include/core/Constants.h"
#include <algorithm>
#include <numeric>

MovingAverageFilter::MovingAverageFilter(size_t windowSize)
    : FilterBase("MovingAverage"), m_windowSize(windowSize), m_window() {
  if (windowSize < Constants::Filters::MIN_MOVING_AVERAGE_WINDOW) {
    m_windowSize = Constants::Filters::MIN_MOVING_AVERAGE_WINDOW;
  } else if (windowSize > Constants::Filters::MAX_MOVING_AVERAGE_WINDOW) {
    m_windowSize = Constants::Filters::MAX_MOVING_AVERAGE_WINDOW;
  }
}

DataPoint MovingAverageFilter::filter(const DataPoint &input) {
  m_window.push_back(input.value);

  while (m_window.size() > m_windowSize) {
    m_window.pop_front();
  }

  float average = calculateAverage();
  return DataPoint(input.timestamp, average);
}

void MovingAverageFilter::reset() { m_window.clear(); }

bool MovingAverageFilter::isReady() const {
  return m_window.size() >= m_windowSize;
}

void MovingAverageFilter::setWindowSize(size_t windowSize) {
  if (windowSize < Constants::Filters::MIN_MOVING_AVERAGE_WINDOW) {
    m_windowSize = Constants::Filters::MIN_MOVING_AVERAGE_WINDOW;
  } else if (windowSize > Constants::Filters::MAX_MOVING_AVERAGE_WINDOW) {
    m_windowSize = Constants::Filters::MAX_MOVING_AVERAGE_WINDOW;
  } else {
    m_windowSize = windowSize;
  }

  m_window.clear();
}

size_t MovingAverageFilter::getWindowSize() const { return m_windowSize; }

size_t MovingAverageFilter::getMemoryUsage() const {
  return m_window.size() * sizeof(float) + sizeof(m_windowSize) +
         sizeof(m_window);
}

float MovingAverageFilter::calculateAverage() const {
  if (m_window.empty()) {
    return 0.0f;
  }

  float sum = std::accumulate(m_window.begin(), m_window.end(), 0.0f);
  return sum / static_cast<float>(m_window.size());
}
