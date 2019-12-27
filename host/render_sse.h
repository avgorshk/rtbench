#ifndef RTBENCH_HOST_RENDER_SSE_H_
#define RTBENCH_HOST_RENDER_SSE_H_

#include <vector>

#include "../light.h"
#include "../sphere.h"

namespace host {
namespace sse {

void Render(const std::vector<Sphere>& spheres,
            const std::vector<Light>& lights,
            std::vector<Vector>& image,
            int w, int h);

} // namespace sse
} // namespace host

#endif // RTBENCH_HOST_RENDER_SSE_H_