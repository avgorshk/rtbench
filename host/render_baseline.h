#ifndef RTBENCH_HOST_RENDER_BASELINE_H_
#define RTBENCH_HOST_RENDER_BASELINE_H_

#include <vector>

#include "../light.h"
#include "../sphere.h"

namespace host {
namespace baseline {

void Render(const std::vector<Sphere>& spheres,
            const std::vector<Light>& lights,
            std::vector<Vector>& image,
            int w, int h);

} // namespace baseline
} // namespace host

#endif // RTBENCH_HOST_RENDER_BASELINE_H_