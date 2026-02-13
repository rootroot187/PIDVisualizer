#include "../../include/network/udpreceiver.h"
#include "../../include/core/Constants.h"
#include "../../include/network/protocolparser.h"

#ifdef _WIN32
// WinSock2 для шиндовс
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
// POSIX-сокеты для линуха
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#include <QDebug>
#include <cstring>
#include <stdexcept>

UdpReceiver::UdpReceiver(ThreadSafeRingBuffer<DataPoint> *buffer,
                         std::function<void(const DataPoint &)> onDataReceived)
    : m_buffer(buffer), m_onDataReceived(onDataReceived), m_running(false)

#ifdef _WIN32

      ,
      m_socket(INVALID_SOCKET)

#else

      ,
      m_socket(-1)

#endif
      ,
      m_packetsReceived(0) {
  if (!m_buffer) {
    throw std::invalid_argument("Buffer cant be null");
  }
}

UdpReceiver::~UdpReceiver() { stop(); }

bool UdpReceiver::start(const std::string &ip, uint16_t port) {
  if (m_running) {
    return false;
  }
  // если поток завершен, то детачим его
  if (m_receiveThread.joinable()) {
    m_receiveThread.detach();
  }
  // инициализируем сокет
  if (!initializeSocket(ip, port)) {
    return false;
  }
  // запускаем поток
  m_running = true;
  try {
    m_receiveThread = std::thread(&UdpReceiver::receiveLoop, this);
  } catch (const std::exception &e) {
    // ошибка создания потока - закрываем сокет и сбрасываем флаг
    m_running = false;
    closeSocket();
    return false;
  }

  return true;
}

void UdpReceiver::stop() {
  if (!m_running) {
    return;
  }
  // останавливаем прием
  m_running = false;
  closeSocket();

  // ждем завершения потока
  if (m_receiveThread.joinable()) {
    m_receiveThread.join();
  }
}

bool UdpReceiver::isRunning() const { return m_running; }

size_t UdpReceiver::getPacketsReceived() const { return m_packetsReceived; }

void UdpReceiver::resetStatistics() { m_packetsReceived = 0; }

void UdpReceiver::receiveLoop() {

  int packetCount = 0;
  // буфер для приема данных
  uint8_t buffer[Constants::Network::UDP_BUFFER_SIZE];

  // бесконечный цикл приема данных
  while (m_running) {

#ifdef _WIN32 // если шиндовс
    if (m_socket == INVALID_SOCKET) {
      break;
    }
#else // если линух
    if (m_socket < 0) {
      break;
    }
#endif

    // структура для адреса отправителя
    struct sockaddr_in senderAddr;
    socklen_t senderLen = sizeof(senderAddr);

#ifdef _WIN32
    int received =
        recvfrom(m_socket, reinterpret_cast<char *>(buffer), sizeof(buffer), 0,
                 reinterpret_cast<struct sockaddr *>(&senderAddr), &senderLen);
#else
    ssize_t received = recvfrom(m_socket, buffer, sizeof(buffer), 0,
                                (struct sockaddr *)&senderAddr, &senderLen);
#endif

    // проверка: получили ли мы данные?
    if (received < 0) {
      // ошибка приема (может быть временная или сокет закрыт)
      // если сокет закрыт, выходим из цикла
#ifdef _WIN32 // если шиндовс
      if (m_socket == INVALID_SOCKET || !m_running) {
        break;
      }
#else // если линух
      if (m_socket < 0 || !m_running) {
        break;
      }
#endif
      // чтош, продолжаем цикл
      continue;
    }

    // правильный ли размер пакета?
    if (received == Constants::Network::DATA_PACKET_SIZE) {
      packetCount++;

      try {
        DataPoint point = ProtocolParser::parseDataPacket(buffer, received);
        m_buffer->push(point);

        if (m_onDataReceived) {
          m_onDataReceived(point);
        }

        // атомарно обновляем статистику
        m_packetsReceived++;

      } catch (const std::exception &e) {
        continue;
      }
    }
  }
}

bool UdpReceiver::initializeSocket(const std::string &ip, uint16_t port) {
  // инициализируем сокет
#ifdef _WIN32
  // для шиндовс необходимо проинициализировать winsock
  static bool winsockInitialized = false;
  if (!winsockInitialized) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
      return false;
    }
    winsockInitialized = true;
  }
  m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (m_socket == INVALID_SOCKET) {
    return false;
  }

  // настройки сокета
  BOOL reuse = TRUE;
  if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR,
                 reinterpret_cast<const char *>(&reuse),
                 sizeof(reuse)) == SOCKET_ERROR) {
    closesocket(m_socket);
    m_socket = INVALID_SOCKET;
    return false;
  }
  DWORD timeoutMs = 100;
  if (setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO,
                 reinterpret_cast<const char *>(&timeoutMs),
                 sizeof(timeoutMs)) == SOCKET_ERROR) {
    closesocket(m_socket);
    m_socket = INVALID_SOCKET;
    return false;
  }
#else
  // настройка сокета для линуха
  m_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (m_socket < 0) {
    return false;
  }

  int reuse = 1;
  if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
      0) {
    close(m_socket);
    m_socket = -1;
    return false;
  }

  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 100000;
  if (setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) <
      0) {
    close(m_socket);
    m_socket = -1;
    return false;
  }
#endif

  // настройка адреса
  struct sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET;                    // IPv4
  addr.sin_port = htons(port);                  // порт
  addr.sin_addr.s_addr = inet_addr(ip.c_str()); // IP

  // правильный ли IP адрес?
  if (addr.sin_addr.s_addr == INADDR_NONE) {
    // ошибка: неправильный IP адрес
#ifdef _WIN32
    closesocket(m_socket);
    m_socket = -1;
#else
    close(m_socket);
    m_socket = -1;
#endif
    return false;
  }

  // привязываем сокет к адресу
  if (bind(m_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    // ошибка: не удалось привязать
#ifdef _WIN32
    closesocket(m_socket);
    m_socket = INVALID_SOCKET;
#else
    close(m_socket);
    m_socket = -1;
#endif
    return false;
  }

  return true;
}

void UdpReceiver::closeSocket() {
  // закрываем сокет
#ifdef _WIN32
  if (m_socket != INVALID_SOCKET) {
    closesocket(m_socket);
    m_socket = INVALID_SOCKET;
  }
#else
  if (m_socket >= 0) {
    close(m_socket);
    m_socket = -1;
  }
#endif
}
