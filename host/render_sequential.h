#ifndef RTBENCH_HOST_RENDER_SEQUENTIAL_H_
#define RTBENCH_HOST_RENDER_SEQUENTIAL_H_

#include <vector>

#include "../light.h"
#include "../sphere.h"

namespace host {
namespace sequential {

void Render(const std::vector<Sphere>& spheres,
            const std::vector<Light>& lights,
            std::vector<Vector>& image,
            int w, int h);

} // namespace sequential
} // namespace host

#endif // RTBENCH_HOST_RENDER_SEQUENTIAL_H_