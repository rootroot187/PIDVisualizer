#include "../../include/core/processmemory.h"
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>

#ifdef _WIN32
#include <psapi.h>
#include <windows.h>
#pragma comment(lib, "psapi.lib")
#endif

namespace ProcessMemory {

uint64_t getCurrentProcessRSSBytes() {
#ifdef __linux__
  // /proc/self/status содержит VmRSS в кб
  std::ifstream status("/proc/self/status");
  if (!status.is_open()) {
    return 0;
  }
  std::string line;
  while (std::getline(status, line)) {
    if (line.compare(0, 6, "VmRSS:") == 0) {
      size_t pos = 6;
      while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) {
        ++pos;
      }
      unsigned long kb = 0;
      const char *start = line.c_str() + pos;
      char *end = nullptr;
      kb = std::strtoul(start, &end, 10);
      if (end != start) {
        status.close();
        return static_cast<uint64_t>(kb) * 1024ULL;
      }
      break;
    }
  }
  return 0;

#elif defined(_WIN32)
  PROCESS_MEMORY_COUNTERS pmc = {};
  pmc.cb = sizeof(pmc);
  if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
    return static_cast<uint64_t>(pmc.WorkingSetSize);
  }
  return 0;

#else
  (void)0; // suppress unused warning
  return 0;
#endif
}

double getCurrentProcessRSSMegabytes() {
  uint64_t bytes = getCurrentProcessRSSBytes();
  if (bytes == 0) {
    return -1.0;
  }
  return static_cast<double>(bytes) / (1024.0 * 1024.0);
}

} // namespace ProcessMemory
