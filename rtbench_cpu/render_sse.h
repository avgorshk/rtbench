#ifndef RTBENCH_RENDER_SSE_H_
#define RTBENCH_RENDER_SSE_H_

#include <vector>

#include "common/light.h"
#include "common/sphere.h"

namespace sse {

void Render(const std::vector<Sphere>& spheres,
            const std::vector<Light>& lights,
            std::vector<Vector>& image,
            int w, int h);

} // namespace sse

#endif // RTBENCH_RENDER_SSE_H_