#ifndef GRAPHMANAGER_H
#define GRAPHMANAGER_H

#include "../core/datapoint.h"
#include "../core/threadsaferingbuffer.h"
#include <QWidget>
#include <memory>
#include <qcustomplot.h>
#include <vector>

class QTabWidget;

// структура для хранения пары график-буфер
struct GraphSeries {
  QCPGraph *graph;
  ThreadSafeRingBuffer<DataPoint> *buffer;
  QString name;

  GraphSeries(QCPGraph *g, ThreadSafeRingBuffer<DataPoint> *b, const QString &n)
      : graph(g), buffer(b), name(n) {}
};

class GraphManager : public QObject {
  Q_OBJECT

public:
  explicit GraphManager(QWidget *parent = nullptr);
  ~GraphManager();

  // настройка графиков
  void setupGraphs(QWidget *containerWidget);

  // обновление графиков
  void updateGraph(const std::vector<GraphSeries> &series, size_t maxSamples);
  void updateSpectrum(const std::vector<GraphSeries> &series, bool isRunning);

  // управление видимостью серий
  void setSeriesVisible(const QString &name, bool visible);

  // геттеры для доступа к графикам
  QCustomPlot *getSignalPlot() const { return m_plot; }
  QCustomPlot *getSpectrumPlot() const { return m_spectrumPlot; }
  QWidget *getTabWidget() const { return m_tabWidget; }
  QCPGraph *getRawDataGraph() const { return m_rawDataGraph; }
  QCPGraph *getMovingAvgGraph() const { return m_movingAvgGraph; }
  QCPGraph *getMedianGraph() const { return m_medianGraph; }
  QCPGraph *getExponentialGraph() const { return m_exponentialGraph; }
  QCPGraph *getKalmanGraph() const { return m_kalmanGraph; }

private:
  bool readBufferData(ThreadSafeRingBuffer<DataPoint> *buffer,
                      std::vector<DataPoint> &data);
  void addPointsToGraph(QCPGraph *graph, const std::vector<DataPoint> &data);
  void removeOldPoints(const std::vector<QCPGraph *> &graphs,
                       size_t maxSamples);

  QCustomPlot *m_plot;
  QCustomPlot *m_spectrumPlot;
  QTabWidget *m_tabWidget;

  // серии данных на графике сигнала
  QCPGraph *m_rawDataGraph;
  QCPGraph *m_movingAvgGraph;
  QCPGraph *m_medianGraph;
  QCPGraph *m_exponentialGraph;
  QCPGraph *m_kalmanGraph;

  // серии данных на графике спектра
  QCPGraph *m_rawSpectrumGraph;
  QCPGraph *m_movingAvgSpectrumGraph;
  QCPGraph *m_medianSpectrumGraph;
  QCPGraph *m_exponentialSpectrumGraph;
  QCPGraph *m_kalmanSpectrumGraph;
};

#endif // GRAPHMANAGER_H
