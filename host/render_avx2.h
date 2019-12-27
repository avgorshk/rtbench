#ifndef RTBENCH_HOST_RENDER_AVX2_H_
#define RTBENCH_HOST_RENDER_AVX2_H_

#include <vector>

#include "../light.h"
#include "../sphere.h"

namespace host {
namespace avx2 {

void Render(const std::vector<Sphere>& spheres,
            const std::vector<Light>& lights,
            std::vector<Vector>& image,
            int w, int h);

} // namespace avx2
} // namespace host

#endif // RTBENCH_HOST_RENDER_AVX2_H_