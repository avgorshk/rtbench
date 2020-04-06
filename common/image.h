#ifndef RTBENCH_COMMON_IMAGE_H_
#define RTBENCH_COMMON_IMAGE_H_

#include <algorithm>
#include <vector>

#include <assert.h>

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/stb_image_write.h"

#include "vector.h"

namespace image {

inline bool Load(const char* filename, int& w, int& h,
                 std::vector<Vector>& image) {
  assert(filename != nullptr);

  int comp = -1;
  uint8_t* data = stbi_load(filename, &w, &h, &comp, 0);
  if (data == nullptr) {
    return false;
  } else if (comp != 3) {
    stbi_image_free(data);
    return false;
  }
  
  image.resize(w * h);
  for (int i = 0; i < w * h; ++i) {
    image[i] = Vector(data[3 * i + 0],
                      data[3 * i + 1],
                      data[3 * i + 2]) / 255.0f;
  }
  
  stbi_image_free(data);
  return true;
}

inline std::vector<Vector> Normalize(const std::vector<Vector>& image) {
  std::vector<Vector> normalized(image.size(), 0.0f);

  for (size_t i = 0; i < image.size(); ++i) {
    Vector pixel = image[i];
    float max = std::max(pixel.x(), std::max(pixel.y(), pixel.z()));
    if (max > 1) pixel = pixel * (1.0f / max);
    normalized[i] = Vector(std::max(0.0f, std::min(1.0f, pixel.x())),
                           std::max(0.0f, std::min(1.0f, pixel.y())),
                           std::max(0.0f, std::min(1.0f, pixel.z())));
  }

  return normalized;
}

inline bool SavePng(const char* filename, int w, int h,
                    const std::vector<Vector>& image) {
  assert(filename != nullptr);
  assert(image.size() == w * h);

  std::vector<Vector> normalized = Normalize(image);

  std::vector<uint8_t> data(3 * w * h, 0);
  for (int i = 0; i < w * h; ++i) {
    Vector& pixel = normalized[i];
    data[3 * i + 0] = static_cast<uint8_t>(255.0f * pixel.x());
    data[3 * i + 1] = static_cast<uint8_t>(255.0f * pixel.y());
    data[3 * i + 2] = static_cast<uint8_t>(255.0f * pixel.z());
  }

  int status = stbi_write_png(filename, w, h, 3 /*RGB*/, data.data(),
                              3 * w * sizeof(uint8_t));
  return status == 1 ? true : false;
}

inline bool Compare(const std::vector<Vector>& output, const char* reference) {
  std::vector<Vector> image;
  int w = 0, h = 0;
  bool loaded = image::Load(reference, w, h, image);
  if (!loaded) {
    std::cout << "Reference output file was not found: " <<
      reference << std::endl;
    return false;
  }

  std::vector<Vector> normalized = Normalize(output);

  if (normalized.size() != image.size()) {
    return false;
  }

  const float kEps = 2.0f / 255.0f;
  for (size_t i = 0; i < normalized.size(); ++i) {
    float diff = (normalized[i] - image[i]).norm();
    if (diff > kEps) {
      std::cout << "(" << i / w << ", " << i % w << ")...";
      return false;
    }
  }

  return true;
}

} // image

#endif // RTBENCH_COMMON_IMAGE_H_