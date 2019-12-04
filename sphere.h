#ifndef RTBENCH_SPHERE_H_
#define RTBENCH_SPHERE_H_

#include "material.h"

struct Sphere {
 public:
  Sphere(const Vector& center, float radius, const Material& material)
      : material_(material), center_(center), radius_(radius) {}

  bool RayIntersect(const Vector& orig, const Vector& dir, float& t0) const {
    Vector L = center_ - orig;
    float tca = L * dir;
    float d2 = L * L - tca * tca;
    if (d2 > radius_* radius_) {
      return false;
    }
    float thc = sqrtf(radius_ * radius_ - d2);
    t0 = tca - thc;
    float t1 = tca + thc;
    if (t0 < 0) {
      t0 = t1;
    }
    if (t0 < 0) {
      return false;
    }
    return true;
  }

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

#endif // RTBENCH_SPHERE_H_