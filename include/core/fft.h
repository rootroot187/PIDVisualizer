#ifndef FFT_H
#define FFT_H

#include <cmath>
#include <complex>
#include <vector>

/**
 * @brief простая реализация FFT для анализа спектра
 */
namespace FFT {

/**
 * @brief вычислить FFT для вещественного сигнала
 *
 * @param input вектор значений float
 * @return вектор комплексных чисел - спектр

 */
std::vector<std::complex<double>> computeFFT(const std::vector<float> &input);

/**
 * @brief вычислить амплитудный спектр (модуль БПФ)
 *
 * @param input вектор значений float
 * @return вектор амплитуд для каждой частоты
 */
std::vector<double> computeAmplitudeSpectrum(const std::vector<float> &input);

/**
 * @brief вычислить частоты для спектра гц
 *
 * @param spectrumSize количество точек БПФ
 * @param sampleRate частота дискретизации гц
 * @return вектор частот гц для каждой точки спектра
 */
std::vector<double> computeFrequencies(size_t spectrumSize, double sampleRate);

/**
 * @brief найти ближайшую степень двойки >= n
 *
 * @param n исходное число
 * @return ближайшая степень двойки (2^k >= n)
 */
size_t nextPowerOfTwo(size_t n);

} // namespace FFT

#endif // FFT_H
