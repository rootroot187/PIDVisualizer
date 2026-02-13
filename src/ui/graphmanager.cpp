#include "../../include/ui/graphmanager.h"
#include "../../include/core/fft.h"
#include <QColor>
#include <QDebug>
#include <QFont>
#include <QHBoxLayout>
#include <QPen>
#include <QTabWidget>
#include <algorithm>
#include <cmath>

GraphManager::GraphManager(QWidget *parent)
    : QObject(parent), m_plot(nullptr), m_spectrumPlot(nullptr),
      m_tabWidget(nullptr), m_rawDataGraph(nullptr), m_movingAvgGraph(nullptr),
      m_medianGraph(nullptr), m_exponentialGraph(nullptr),
      m_kalmanGraph(nullptr), m_rawSpectrumGraph(nullptr),
      m_movingAvgSpectrumGraph(nullptr), m_medianSpectrumGraph(nullptr),
      m_exponentialSpectrumGraph(nullptr), m_kalmanSpectrumGraph(nullptr) {}

GraphManager::~GraphManager() {}

void GraphManager::setupGraphs(QWidget *containerWidget) {
  if (!containerWidget) {
    qWarning("GraphManager: containerWidget is null");
    return;
  }

  m_tabWidget = new QTabWidget(containerWidget);
  if (!m_tabWidget) {
    qFatal("Failed to create QTabWidget");
    return;
  }
  m_tabWidget->setMinimumSize(400, 300);

  // график сигнала
  m_plot = new QCustomPlot(m_tabWidget);
  if (!m_plot) {
    qFatal("Failed to create QCustomPlot widget");
    return;
  }

  m_plot->xAxis->setLabel("Время (мс)");
  m_plot->yAxis->setLabel("Значение");
  m_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

  // добавляем данные
  m_rawDataGraph = m_plot->addGraph();
  m_movingAvgGraph = m_plot->addGraph();
  m_medianGraph = m_plot->addGraph();
  m_exponentialGraph = m_plot->addGraph();
  m_kalmanGraph = m_plot->addGraph();

  if (!m_rawDataGraph || !m_movingAvgGraph || !m_medianGraph ||
      !m_exponentialGraph || !m_kalmanGraph) {
    qWarning("Failed to create graph series");
    return;
  }

  // цвета серий данных
  m_rawDataGraph->setPen(QPen(QColor(255, 0, 0), 2));
  m_movingAvgGraph->setPen(QPen(QColor(0, 255, 0), 2));
  m_medianGraph->setPen(QPen(QColor(0, 0, 255), 2));
  m_exponentialGraph->setPen(QPen(QColor(255, 0, 255), 2));
  m_kalmanGraph->setPen(QPen(QColor(0, 255, 255), 2));

  m_rawDataGraph->setName("Исходные данные");
  m_movingAvgGraph->setName("Moving Average");
  m_medianGraph->setName("Median");
  m_exponentialGraph->setName("Exponential");
  m_kalmanGraph->setName("Kalman");

  m_plot->legend->setVisible(true);
  m_plot->legend->setFont(QFont("Helvetica", 9));
  m_plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft |
                                                              Qt::AlignTop);

  m_plot->xAxis->setRange(0, 1000);
  m_plot->yAxis->setRange(-10, 10);

  m_tabWidget->addTab(m_plot, "Сигнал");

  m_plot->setVisible(true);

  // создаем график спектра
  m_spectrumPlot = new QCustomPlot(m_tabWidget);
  if (!m_spectrumPlot) {
    qWarning("Failed to create spectrum plot widget");
    return;
  }

  m_spectrumPlot->xAxis->setLabel("Частота (Гц)");
  m_spectrumPlot->yAxis->setLabel("Амплитуда");
  m_spectrumPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

  m_rawSpectrumGraph = m_spectrumPlot->addGraph();
  m_movingAvgSpectrumGraph = m_spectrumPlot->addGraph();
  m_medianSpectrumGraph = m_spectrumPlot->addGraph();
  m_exponentialSpectrumGraph = m_spectrumPlot->addGraph();
  m_kalmanSpectrumGraph = m_spectrumPlot->addGraph();

  m_rawSpectrumGraph->setPen(QPen(QColor(255, 0, 0), 2));
  m_movingAvgSpectrumGraph->setPen(QPen(QColor(0, 255, 0), 2));
  m_medianSpectrumGraph->setPen(QPen(QColor(0, 0, 255), 2));
  m_exponentialSpectrumGraph->setPen(QPen(QColor(255, 0, 255), 2));
  m_kalmanSpectrumGraph->setPen(QPen(QColor(0, 255, 255), 2));

  m_rawSpectrumGraph->setName("Исходные данные");
  m_movingAvgSpectrumGraph->setName("Moving Average");
  m_medianSpectrumGraph->setName("Median");
  m_exponentialSpectrumGraph->setName("Exponential");
  m_kalmanSpectrumGraph->setName("Kalman");

  m_spectrumPlot->legend->setVisible(true);
  m_spectrumPlot->legend->setFont(QFont("Helvetica", 9));
  m_spectrumPlot->axisRect()->insetLayout()->setInsetAlignment(
      0, Qt::AlignLeft | Qt::AlignTop);

  m_spectrumPlot->xAxis->setRange(0, 25);
  m_spectrumPlot->yAxis->setRange(0, 100);

  // добавляем график спектра в tabWidget
  m_tabWidget->addTab(m_spectrumPlot, "Спектр (БПФ)");

  // убеждаемся, что tabWidget и графики видны
  m_tabWidget->setVisible(true);
  m_spectrumPlot->setVisible(true);
}

void GraphManager::updateGraph(const std::vector<GraphSeries> &series,
                               size_t maxSamples) {
  if (!m_plot) {
    return;
  }

  // проверяем валидность всех графиков и буферов
  bool allValid = true;
  for (const auto &s : series) {
    if (!s.graph || !s.buffer) {
      allValid = false;
      break;
    }
  }

  if (!allValid) {
    return;
  }

  try {
    // читаем данные из буферов и добавляем на графики
    for (const auto &s : series) {
      std::vector<DataPoint> data;
      if (readBufferData(s.buffer, data)) {
        addPointsToGraph(s.graph, data);
      }
    }

    // собираем все графики для удаления старых точек
    std::vector<QCPGraph *> allGraphs;
    for (const auto &s : series) {
      if (s.graph) {
        allGraphs.push_back(s.graph);
      }
    }

    removeOldPoints(allGraphs, maxSamples);
    m_plot->rescaleAxes();
    m_plot->replot();
  } catch (const std::exception &e) {
    qWarning() << "[GraphManager::updateGraph] - ошибка:" << e.what();
  } catch (...) {
    qWarning() << "[GraphManager::updateGraph] - неизвестная ошибка";
  }
}

void GraphManager::updateSpectrum(const std::vector<GraphSeries> &series,
                                  bool isRunning) {
  if (!m_spectrumPlot || !isRunning || !m_rawDataGraph) {
    return;
  }

  constexpr size_t MIN_FFT_SIZE = 64;
  constexpr size_t MAX_FFT_SIZE = 1024;

  // оценка частоты дискретизации по данным на графике
  double sampleRate = 50.0;
  {
    auto rawData = m_rawDataGraph->data();
    if (rawData && rawData->size() >= 2) {
      uint32_t totalTime = 0;
      int validPairs = 0;

      auto itPrev = rawData->constBegin();
      for (auto it = rawData->constBegin() + 1; it != rawData->constEnd();
           ++it) {
        uint32_t tsPrev = static_cast<uint32_t>(itPrev->key);
        uint32_t tsCur = static_cast<uint32_t>(it->key);
        uint32_t dt = tsCur - tsPrev;
        if (dt > 0 && dt < 10000) {
          totalTime += dt;
          ++validPairs;
        }
        itPrev = it;
      }

      if (validPairs > 0 && totalTime > 0) {
        double avgIntervalMs = static_cast<double>(totalTime) / validPairs;
        if (avgIntervalMs > 0.0) {
          sampleRate = 1000.0 / avgIntervalMs;
        }
      }
    }
  }

  // вычисление спектра одного сигнала по данным из графика
  auto computeAndPlotSpectrumFromGraph = [&](QCPGraph *sourceGraph,
                                             QCPGraph *spectrumGraph) {
    if (!sourceGraph || !spectrumGraph) {
      return;
    }

    auto data = sourceGraph->data();
    if (!data || data->size() < MIN_FFT_SIZE) {
      return;
    }

    const int count =
        static_cast<int>(std::min<size_t>(MAX_FFT_SIZE, data->size()));

    std::vector<float> values;
    values.reserve(count);
    auto it = data->constEnd();
    for (int i = 0; i < count; ++i) {
      --it;
      values.push_back(static_cast<float>(it->value));
      if (it == data->constBegin()) {
        break;
      }
    }

    if (values.size() < MIN_FFT_SIZE) {
      return;
    }

    // вычисляем амплитудный спектр
    std::vector<double> amplitudes = FFT::computeAmplitudeSpectrum(values);
    std::vector<double> frequencies =
        FFT::computeFrequencies(amplitudes.size(), sampleRate);

    // очищаем старые данные спектра
    spectrumGraph->data()->clear();

    // добавляем только первую половину спектра
    size_t spectrumSize = amplitudes.size() / 2;
    for (size_t i = 0; i < spectrumSize; ++i) {
      spectrumGraph->addData(frequencies[i], amplitudes[i]);
    }
  };

  // вычисляем спектр для всех серий
  computeAndPlotSpectrumFromGraph(m_rawDataGraph, m_rawSpectrumGraph);
  computeAndPlotSpectrumFromGraph(m_movingAvgGraph, m_movingAvgSpectrumGraph);
  computeAndPlotSpectrumFromGraph(m_medianGraph, m_medianSpectrumGraph);
  computeAndPlotSpectrumFromGraph(m_exponentialGraph,
                                  m_exponentialSpectrumGraph);
  computeAndPlotSpectrumFromGraph(m_kalmanGraph, m_kalmanSpectrumGraph);

  m_spectrumPlot->rescaleAxes();
  m_spectrumPlot->replot();
}

void GraphManager::setSeriesVisible(const QString &name, bool visible) {
  QCPGraph *graph = nullptr;
  QCPGraph *spectrumGraph = nullptr;

  if (name == "MovingAverage") {
    graph = m_movingAvgGraph;
    spectrumGraph = m_movingAvgSpectrumGraph;
  } else if (name == "Median") {
    graph = m_medianGraph;
    spectrumGraph = m_medianSpectrumGraph;
  } else if (name == "Exponential") {
    graph = m_exponentialGraph;
    spectrumGraph = m_exponentialSpectrumGraph;
  } else if (name == "Kalman") {
    graph = m_kalmanGraph;
    spectrumGraph = m_kalmanSpectrumGraph;
  }

  if (graph) {
    graph->setVisible(visible);
  }
  if (spectrumGraph) {
    spectrumGraph->setVisible(visible);
  }
}

bool GraphManager::readBufferData(ThreadSafeRingBuffer<DataPoint> *buffer,
                                  std::vector<DataPoint> &data) {
  if (!buffer) {
    return false;
  }

  try {
    data = buffer->popAll();
    return true;
  } catch (const std::exception &e) {
    return false;
  } catch (...) {
    return false;
  }
}

void GraphManager::addPointsToGraph(QCPGraph *graph,
                                    const std::vector<DataPoint> &data) {
  if (!graph) {
    return;
  }

  for (const auto &point : data) {
    graph->addData(point.timestamp, point.value);
  }
}

void GraphManager::removeOldPoints(const std::vector<QCPGraph *> &graphs,
                                   size_t maxSamples) {
  if (graphs.empty() || !graphs[0]) {
    return;
  }

  QCPGraph *referenceGraph = graphs[0];
  if (referenceGraph->dataCount() > maxSamples) {
    double removeKey =
        referenceGraph->dataMainKey(referenceGraph->dataCount() - maxSamples);

    for (QCPGraph *graph : graphs) {
      if (graph) {
        graph->data()->removeBefore(removeKey);
      }
    }
  }
}
