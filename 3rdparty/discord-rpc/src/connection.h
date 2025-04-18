#pragma once

// This is to wrap the platform specific kinds of connect/read/write.

#include <cstdlib>

namespace discord_rpc {

// not really connectiony, but need per-platform
int GetProcessId();

struct BaseConnection {
  static BaseConnection *Create();
  static void Destroy(BaseConnection *&);
  bool isOpen { false };
  bool Open();
  bool Close();
  bool Write(const void *data, size_t length);
  bool Read(void *data, size_t length);
};

} // namespace discord_rpc
