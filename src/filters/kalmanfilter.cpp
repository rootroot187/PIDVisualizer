#include "../../include/filters/kalmanfilter.h"
#include "../../include/core/Constants.h"
#include <algorithm>

KalmanFilter::KalmanFilter(double q, double r, double p)
    : FilterBase("Kalman"), m_q(q), m_r(r), m_p(p), m_x(0.0f), m_v(0.0f),
      m_a(0.0f), m_prevX(0.0f), m_prevV(0.0f), m_prevTimestamp(0),
      m_initialized(false) {
  // валидация параметров
  if (m_q <= 0.0)
    m_q = Constants::Filters::DEFAULT_KALMAN_Q;
  if (m_r <= 0.0)
    m_r = Constants::Filters::DEFAULT_KALMAN_R;
  if (m_p <= 0.0)
    m_p = Constants::Filters::DEFAULT_KALMAN_P;
}

DataPoint KalmanFilter::filter(const DataPoint &input) {
  if (!m_initialized) {
    m_x = input.value;
    m_v = 0.0f;
    m_a = 0.0f;
    m_prevX = input.value;
    m_prevV = 0.0f;
    m_prevTimestamp = input.timestamp;
    m_initialized = true;
    return DataPoint(input.timestamp, m_x);
  }

  uint32_t dt = input.timestamp - m_prevTimestamp;
  if (dt == 0)
    dt = 1;
  double dt_sec = static_cast<double>(dt) / 1000.0;

  float observedV = (input.value - m_prevX) / static_cast<float>(dt);
  float observedA = (observedV - m_prevV) / static_cast<float>(dt);

  float alphaV = 0.5f;
  float alphaA = 0.3f;

  m_v = alphaV * observedV + (1.0f - alphaV) * m_v;
  m_a = alphaA * observedA + (1.0f - alphaA) * m_a;

  double x_pred = static_cast<double>(m_x) + static_cast<double>(m_v) * dt_sec +
                  0.5 * static_cast<double>(m_a) * dt_sec * dt_sec;

  double p_pred = m_p + m_q * dt_sec;

  double k = p_pred / (p_pred + m_r);

  double measurement = static_cast<double>(input.value);
  double x_new = x_pred + k * (measurement - x_pred);

  double p_new = (1.0 - k) * p_pred;

  m_x = static_cast<float>(x_new);
  m_p = p_new;
  m_prevX = input.value;
  m_prevV = m_v;
  m_prevTimestamp = input.timestamp;

  return DataPoint(input.timestamp, m_x);
}

void KalmanFilter::reset() {
  m_x = 0.0f;
  m_v = 0.0f;
  m_a = 0.0f;
  m_prevX = 0.0f;
  m_prevV = 0.0f;
  m_prevTimestamp = 0;
  m_p = Constants::Filters::DEFAULT_KALMAN_P;
  m_initialized = false;
}

bool KalmanFilter::isReady() const { return m_initialized; }

void KalmanFilter::setQ(double q) {
  if (q > 0.0) {
    m_q = q;
    reset();
  }
}

double KalmanFilter::getQ() const { return m_q; }

void KalmanFilter::setR(double r) {
  if (r > 0.0) {
    m_r = r;
    reset();
  }
}

double KalmanFilter::getR() const { return m_r; }

void KalmanFilter::setP(double p) {
  if (p > 0.0) {
    m_p = p;
    reset();
  }
}

double KalmanFilter::getP() const { return m_p; }

void KalmanFilter::setParameters(double q, double r, double p) {
  bool needReset = false;

  if (q > 0.0 && q != m_q) {
    m_q = q;
    needReset = true;
  }

  if (r > 0.0 && r != m_r) {
    m_r = r;
    needReset = true;
  }

  if (p > 0.0 && p != m_p) {
    m_p = p;
    needReset = true;
  }

  if (needReset) {
    reset();
  }
}

size_t KalmanFilter::getMemoryUsage() const {
  return sizeof(m_q) + sizeof(m_r) + sizeof(m_p) + sizeof(m_x) + sizeof(m_v) +
         sizeof(m_a) + sizeof(m_prevX) + sizeof(m_prevV) +
         sizeof(m_prevTimestamp) + sizeof(m_initialized);
}
