#ifndef RTBENCH_COMMON_SPHERE_H_
#define RTBENCH_COMMON_SPHERE_H_

#include "material.h"

class Sphere {
 public:
  Sphere(const Vector& center, float radius, const Material& material)
    : material_(material), center_(center), radius_(radius) {}

  Material material() const {
    return material_;
  }

  Vector center() const {
    return center_;
  }

  float radius() const {
    return radius_;
  }

private:
  Material material_;
  Vector center_;
  float radius_;
};

#endif // RTBENCH_COMMON_SPHERE_H_