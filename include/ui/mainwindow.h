#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QTimer>
#include <memory>
#include <qcustomplot.h>
#include <vector>

class QLineEdit;
class QPushButton;
class QSpinBox;
class QDoubleSpinBox;
class QGroupBox;
class QStatusBar;
class QLabel;

namespace Ui {
class MainWindow;
}

#include "../core/Constants.h"
#include "../core/datapoint.h"
#include "../core/threadsaferingbuffer.h"
#include "../processing/dataprocessor.h"
#include "cyclictargetcontroller.h"
#include "graphmanager.h"
#include "networkcontroller.h"
#include "statusbarmanager.h"

class MovingAverageFilter;
class MedianFilter;
class ExponentialFilter;
class KalmanFilter;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void onSendButtonClicked();
  void onStartStopButtonClicked();
  void updateGraph();

private:
  void setupComponents();
  void setupFilters();
  void setupTimer();
  void connectSignals();

  // компоненты приложения
  std::unique_ptr<NetworkController> m_networkController;
  std::unique_ptr<GraphManager> m_graphManager;
  std::unique_ptr<DataProcessor> m_dataProcessor;
  std::unique_ptr<CyclicTargetController> m_cyclicTargetController;
  std::unique_ptr<StatusBarManager> m_statusBarManager;

  // буферы для данных (нужны для фильтров и графиков)
  std::unique_ptr<ThreadSafeRingBuffer<DataPoint>> m_rawDataDisplayBuffer;
  std::unique_ptr<ThreadSafeRingBuffer<DataPoint>> m_rawBufferMovingAvg;
  std::unique_ptr<ThreadSafeRingBuffer<DataPoint>> m_rawBufferMedian;
  std::unique_ptr<ThreadSafeRingBuffer<DataPoint>> m_rawBufferExponential;
  std::unique_ptr<ThreadSafeRingBuffer<DataPoint>> m_rawBufferKalman;

  // буферы для отображения (получаем от DataProcessor)
  ThreadSafeRingBuffer<DataPoint> *m_movingAvgBuffer;
  ThreadSafeRingBuffer<DataPoint> *m_medianBuffer;
  ThreadSafeRingBuffer<DataPoint> *m_exponentialBuffer;
  ThreadSafeRingBuffer<DataPoint> *m_kalmanBuffer;

  // фильтры для настройки параметров из GUI
  std::unique_ptr<MovingAverageFilter> m_movingAvgFilter;
  std::unique_ptr<MedianFilter> m_medianFilter;
  std::unique_ptr<ExponentialFilter> m_exponentialFilter;
  std::unique_ptr<KalmanFilter> m_kalmanFilter;

  QTimer *m_updateTimer;

  bool m_isRunning;
  size_t m_maxSamples;

  Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
