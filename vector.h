#ifndef RTBENCH_VECTOR_H_
#define RTBENCH_VECTOR_H_

#include <cmath>

class Vector {
 public:
  Vector() : x_(0.0f), y_(0.0f), z_(0.0f), w_(0.0f) {}
  Vector(float x) : x_(x), y_(0.0f), z_(0.0f), w_(0.0f) {}
  Vector(float x, float y) : x_(x), y_(y), z_(0.0f), w_(0.0f) {}
  Vector(float x, float y, float z) : x_(x), y_(y), z_(z), w_(0.0f) {}
  Vector(float x, float y, float z, float w) : x_(x), y_(y), z_(z), w_(w) {}
  
  float x() const {
    return x_;
  }
  
  float y() const {
    return y_;
  }
  
  float z() const {
    return z_;
  }

  float w() const {
    return w_;
  }
  
  Vector& operator=(const Vector& r) {
    x_ = r.x_;
    y_ = r.y_;
    z_ = r.z_;
    w_ = r.w_;
    return *this;
  }
  
  Vector operator-() const {
    return Vector(-x_, -y_, -z_, -w_);
  }

  Vector operator+(const Vector& r) const {
    return Vector(x_ + r.x_, y_ + r.y_, z_ + r.z_, w_ + r.w_);
  }
  
  Vector operator-(const Vector& r) const {
    return Vector(x_ - r.x_, y_ - r.y_, z_ - r.z_, w_ - r.w_);
  }
  
  float operator*(const Vector& r) const {
    return x_ * r.x_ + y_ * r.y_ + z_ * r.z_ + w_ * r.w_;
  }
  
  Vector operator*(float r) const {
    return Vector(x_ * r, y_ * r, z_ * r, w_ * r);
  }

  Vector operator/(float r) const {
    return Vector(x_ / r, y_ / r, z_ / r, w_ / r);
  }
  
  float norm() const {
    return std::sqrt(x_ * x_ + y_ * y_ + z_ * z_ + w_ * w_);
  }
  
  Vector& Normalize() {
    *this = *this * (1.0f / norm());
    return *this;
  }
  
 private:
  float x_;
  float y_;
  float z_;
  float w_;
};

#endif //RTBENCH_VECTOR_H_