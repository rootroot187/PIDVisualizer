#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H

#include "../core/datapoint.h"
#include "../core/threadsaferingbuffer.h"
#include "../network/udpreceiver.h"
#include "../network/udpsender.h"
#include <functional>
#include <memory>
#include <string>

class NetworkController {
public:
  NetworkController();
  ~NetworkController();

  // инициализация с буферами и callback для fan-out
  void initialize(ThreadSafeRingBuffer<DataPoint> *displayBuffer,
                  std::function<void(const DataPoint &)> fanOutCallback);

  // управление приемом данных
  bool startReceiver(const std::string &ip, uint16_t port);
  void stopReceiver();
  bool isReceiverRunning() const;

  // управление отправкой команд
  bool sendCommand(float targetValue, const std::string &ip, uint16_t port);

  // статистика
  size_t getPacketsReceived() const;
  size_t getPacketsSent() const;
  void resetStatistics();

  // геттеры для доступа к компонентам
  UdpReceiver *getReceiver() const { return m_receiver.get(); }
  UdpSender *getSender() const { return m_sender.get(); }

private:
  std::unique_ptr<UdpReceiver> m_receiver;
  std::unique_ptr<UdpSender> m_sender;
  ThreadSafeRingBuffer<DataPoint> *m_displayBuffer;
};

#endif // NETWORKCONTROLLER_H
