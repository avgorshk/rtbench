#ifndef RTBENCH_RENDER_SEQUENTIAL_H_
#define RTBENCH_RENDER_SEQUENTIAL_H_

#include <vector>

#include "common/light.h"
#include "common/sphere.h"

namespace sequential {

void Render(const std::vector<Sphere>& spheres,
            const std::vector<Light>& lights,
            std::vector<Vector>& image,
            int w, int h);

} // namespace sequential

#endif // RTBENCH_RENDER_SEQUENTIAL_H_