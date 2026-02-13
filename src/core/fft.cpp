#include "../../include/core/fft.h"
#include <algorithm>
#include <cmath>

namespace FFT {

size_t nextPowerOfTwo(size_t n) {
  if (n == 0)
    return 1;
  if ((n & (n - 1)) == 0)
    return n; // уже степень двойки

  size_t power = 1;
  while (power < n) {
    power <<= 1;
  }
  return power;
}

std::vector<std::complex<double>> computeFFT(const std::vector<float> &input) {
  if (input.empty()) {
    return {};
  }

  // дополняем до степени двойки нулями
  size_t n = nextPowerOfTwo(input.size());
  std::vector<std::complex<double>> data(n);

  // копируем данные
  for (size_t i = 0; i < input.size(); ++i) {
    data[i] = std::complex<double>(static_cast<double>(input[i]), 0.0);
  }

  // бит-реверс перестановка
  for (size_t i = 1, j = 0; i < n; ++i) {
    size_t bit = n >> 1;
    for (; j & bit; bit >>= 1) {
      j ^= bit;
    }
    j ^= bit;
    if (i < j) {
      std::swap(data[i], data[j]);
    }
  }

  // итеративный алгоритм cooley-tukey fft
  const double PI = 3.14159265358979323846;
  for (size_t len = 2; len <= n; len <<= 1) {
    double angle = -2.0 * PI / static_cast<double>(len);
    std::complex<double> wlen(std::cos(angle), std::sin(angle));

    for (size_t i = 0; i < n; i += len) {
      std::complex<double> w(1.0);
      for (size_t j = 0; j < len / 2; ++j) {
        std::complex<double> u = data[i + j];
        std::complex<double> v = data[i + j + len / 2] * w;
        data[i + j] = u + v;
        data[i + j + len / 2] = u - v;
        w *= wlen;
      }
    }
  }

  return data;
}

std::vector<double> computeAmplitudeSpectrum(const std::vector<float> &input) {
  auto spectrum = computeFFT(input);
  std::vector<double> amplitudes(spectrum.size());

  for (size_t i = 0; i < spectrum.size(); ++i) {
    amplitudes[i] = std::abs(spectrum[i]);
  }

  return amplitudes;
}

std::vector<double> computeFrequencies(size_t spectrumSize, double sampleRate) {
  std::vector<double> frequencies(spectrumSize);
  double freqStep = sampleRate / static_cast<double>(spectrumSize);

  for (size_t i = 0; i < spectrumSize; ++i) {
    frequencies[i] = static_cast<double>(i) * freqStep;
  }

  return frequencies;
}

} // namespace FFT
