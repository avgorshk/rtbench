#ifndef RTBENCH_LIGHT_H_
#define RTBENCH_LIGHT_H_

#include "vector.h"

class Light {
 public:
  Light(const Vector& position, float intensity)
      : position_(position), intensity_(intensity) {}
 
  Vector position() const {
    return position_;
  }

  float intensity() const {
    return intensity_;
  }

 private:
  Vector position_;
  float intensity_;
};

#endif // RTBENCH_LIGHT_H_