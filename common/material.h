#ifndef RTBENCH_COMMON_MATERIAL_H_
#define RTBENCH_COMMON_MATERIAL_H_

#include "vector.h"

class Material {
public:
  Material(float refractive_index, const Vector& albedo,
           const Vector& diffuse_color, float specular_exponent)
      : albedo_(albedo), diffuse_color_(diffuse_color),
        refractive_index_(refractive_index),
        specular_exponent_(specular_exponent) {}
  
  Material() : albedo_(1.0f), diffuse_color_(),
               refractive_index_(1.0f), specular_exponent_(0.0f) {}

  Vector albedo() const {
    return albedo_;
  }

  Vector diffuse_color() const {
    return diffuse_color_;
  }

  float refractive_index() const {
    return refractive_index_;
  }

  float specular_exponent() const {
    return specular_exponent_;
  }

  void SetDiffuseColor(const Vector& diffuse_color) {
    diffuse_color_ = diffuse_color;
  }

private:
  Vector albedo_;
  Vector diffuse_color_;
  float refractive_index_;
  float specular_exponent_;
};

#endif // RTBENCH_COMMON_MATERIAL_H_