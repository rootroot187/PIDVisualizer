#include "../../include/ui/networkcontroller.h"

NetworkController::NetworkController() : m_displayBuffer(nullptr) {}

NetworkController::~NetworkController() { stopReceiver(); }

void NetworkController::initialize(
    ThreadSafeRingBuffer<DataPoint> *displayBuffer,
    std::function<void(const DataPoint &)> fanOutCallback) {
  m_displayBuffer = displayBuffer;

  if (!m_displayBuffer) {
    return;
  }

  // создаем UdpReceiver с коллбеком для веерного вывода
  m_receiver = std::make_unique<UdpReceiver>(m_displayBuffer, fanOutCallback);
  m_sender = std::make_unique<UdpSender>();
}

bool NetworkController::startReceiver(const std::string &ip, uint16_t port) {
  if (!m_receiver) {
    return false;
  }

  return m_receiver->start(ip, port);
}

void NetworkController::stopReceiver() {
  if (m_receiver && m_receiver->isRunning()) {
    m_receiver->stop();
  }
}

bool NetworkController::isReceiverRunning() const {
  return m_receiver && m_receiver->isRunning();
}

bool NetworkController::sendCommand(float targetValue, const std::string &ip,
                                    uint16_t port) {
  if (!m_sender) {
    return false;
  }

  return m_sender->sendCommand(targetValue, ip, port);
}

size_t NetworkController::getPacketsReceived() const {
  return m_receiver ? m_receiver->getPacketsReceived() : 0;
}

size_t NetworkController::getPacketsSent() const {
  return m_sender ? m_sender->getPacketsSent() : 0;
}

void NetworkController::resetStatistics() {
  if (m_receiver) {
    m_receiver->resetStatistics();
  }
  if (m_sender) {
    m_sender->resetStatistics();
  }
}
