#ifndef RTBENCH_RENDER_BASELINE_H_
#define RTBENCH_RENDER_BASELINE_H_

#include <vector>

#include "common/light.h"
#include "common/sphere.h"

namespace baseline {

void Render(const std::vector<Sphere>& spheres,
            const std::vector<Light>& lights,
            std::vector<Vector>& image,
            int w, int h);

} // namespace baseline

#endif // RTBENCH_RENDER_BASELINE_H_