#ifndef RTBENCH_DEVICE_H_
#define RTBENCH_DEVICE_H_

#include <intrin.h>
#include <memory.h>

#include <string>
#include <vector>

#include "host/host.h"

namespace device {

static inline std::string GetHostCPU() {
  int cpu_info[4] = { 0 };
  __cpuid(cpu_info, 0x80000000);

  unsigned n_ex_ids = cpu_info[0];
  char cpu_band_string[0x40] = { 0 };
  for (unsigned i = 0x80000000; i <= n_ex_ids; ++i) {
    __cpuid(cpu_info, i);
    if (i == 0x80000002) {
      memcpy(cpu_band_string, cpu_info, sizeof(cpu_info));
    } else if (i == 0x80000003) {
      memcpy(cpu_band_string + 16, cpu_info, sizeof(cpu_info));
    } else if (i == 0x80000004) {
      memcpy(cpu_band_string + 32, cpu_info, sizeof(cpu_info));
    }
  }

  return cpu_band_string;
}

inline std::vector<std::string> GetDeviceList() {
  return std::vector<std::string>{ GetHostCPU() };
}

inline std::vector<std::string> GetVersionList(int device) {
  if (device == 0) {
    return host::GetVersionList();
  }
  return std::vector<std::string>();
}

inline bool Render(const std::vector<Sphere>& spheres,
                   const std::vector<Light>& lights,
                   std::vector<Vector>& image,
                   int w, int h, int device, int version) {
  if (device == 0) {
    return host::Render(spheres, lights, image, w, h, version);
  }
  return false;
}

} // namespace device

#endif // RTBENCH_DEVICE_H_