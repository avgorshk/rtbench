#ifndef RTBENCH_HOST_HOST_H_
#define RTBENCH_HOST_HOST_H_

#include <string>
#include <vector>

#include "render_avx2.h"
#include "render_baseline.h"
#include "render_sequential.h"
#include "render_sse.h"

namespace host {

inline std::vector<std::string> GetVersionList() {
  return std::vector<std::string>{"Sequential", "Baseline", "SSE", "AVX2"};
}

inline bool Render(const std::vector<Sphere>& spheres,
                   const std::vector<Light>& lights,
                   std::vector<Vector>& image,
                   int w, int h, int version) {
  if (version == 0) {
    sequential::Render(spheres, lights, image, w, h);
    return true;
  } else if (version == 1) {
    baseline::Render(spheres, lights, image, w, h);
    return true;
  } else if (version == 2) {
    sse::Render(spheres, lights, image, w, h);
    return true;
  } else if (version == 3) {
    avx2::Render(spheres, lights, image, w, h);
    return true;
  }
  return false;
}

} // namespace host

#endif // RTBENCH_HOST_HOST_H_