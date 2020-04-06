#ifndef RTBENCH_COMMON_SCENE_H_
#define RTBENCH_COMMON_SCENE_H_

#include <vector>

#include "light.h"
#include "material.h"
#include "sphere.h"
#include "vector.h"

class Scene {
 public:
  Scene(const std::vector<Vector>& input) {
    Material ivory(1.0f,
                  Vector(0.6f, 0.3f, 0.1f, 0.0f),
                  Vector(0.4f, 0.4f, 0.3f),
                  50.0f);
    Material glass(1.5f,
                  Vector(0.0f, 0.5f, 0.1f, 0.8f),
                  Vector(0.6f, 0.7f, 0.8f),
                  125.0f);
    Material red_rubber(1.0f,
                        Vector(0.9f, 0.1f, 0.0f, 0.0f),
                        Vector(0.3f, 0.1f, 0.1f),
                        10.0f);
    Material mirror(1.0f,
                    Vector(0.0f, 10.0f, 0.8f, 0.0f),
                    Vector(1.0f, 1.0f, 1.0f),
                    1425.0f);
    
    spheres_.push_back(Sphere(Vector(-3.0f, 0.0f, -16.0f), 2.0f, ivory));
    spheres_.push_back(Sphere(Vector(-1.0f, -1.5f, -12.0f), 2.0f, glass));
    spheres_.push_back(Sphere(Vector(1.5f, -0.5f, -18.0f), 3.0f, red_rubber));
    spheres_.push_back(Sphere(Vector(7.0f, 5.0f, -18.0f), 4.0f, mirror));

    lights_.push_back(Light(Vector(-20.0f, 20.0f, 20.0f), 1.5f));
    lights_.push_back(Light(Vector(30.0f, 50.0f, -25.0f), 1.8f));
    lights_.push_back(Light(Vector(30.0f, 20.0f, 30.0f), 1.7f));

    image_.resize(input.size());
    std::copy(input.begin(), input.end(), image_.begin());
  }

  const std::vector<Sphere>& GetSpheres() const {
    return spheres_;
  }

  const std::vector<Light>& GetLights() const {
    return lights_;
  }

  std::vector<Vector>& GetImage() {
    return image_;
  }

 private:
  std::vector<Sphere> spheres_;
  std::vector<Light> lights_;
  std::vector<Vector> image_;
};

#endif // RTBENCH_COMMON_SCENE_H_