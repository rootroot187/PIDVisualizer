#ifndef UDPSENDER_H
#define UDPSENDER_H

#include "../core/Constants.h"
#include <cstdint>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using SocketHandle = SOCKET;
#else
using SocketHandle = int;
#endif

/**
 * @brief Класс для отправки команд модели по UDP
 */
class UdpSender {
public:
  /**
   * @brief конструктор
   */
  UdpSender();

  /**
   * @brief деструктор
   */
  ~UdpSender();

  /**
   * @brief отправить команду модели
   *
   * создает пакет из float значения и отправляет модели.
   *
   * @param targetValue целевое значение
   * @param ip IP адрес получателя
   * @param port порт получателя
   * @return true или false
   */
  bool sendCommand(float targetValue,
                   const std::string &ip = Constants::Network::DEFAULT_SEND_IP,
                   uint16_t port = Constants::Network::DEFAULT_SEND_PORT);

  /**
   * @brief проверить, инициализирован ли сокет
   */
  bool isInitialized() const;

  /**
   * @brief получить статистику отправки
   */
  size_t getPacketsSent() const;

  /**
   * @brief сбросить статистику
   */
  void resetStatistics();

private:
  /**
   * @brief инициализация сокета
   * @return true или false
   */
  bool initializeSocket();

  /**
   * @brief Закрытие сокета
   */
  void closeSocket();

  SocketHandle m_socket;
  size_t m_packetsSent;
};

#endif // UDPSENDER_H
