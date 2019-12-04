#ifndef RTBENCH_IMAGE_H_
#define RTBENCH_IMAGE_H_

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

inline bool SavePng(const char* filename, int w, int h,
                    std::vector<Vector>& image) {
  assert(filename != nullptr);
  assert(image.size() == w * h);

  std::vector<uint8_t> data(3 * w * h, 0);
  for (int i = 0; i < w * h; ++i) {
    Vector& pixel = image[i];
    float max = std::max(pixel.x(), std::max(pixel.y(), pixel.z()));
    if (max > 1) pixel = pixel * (1.0f / max);
    data[3 * i + 0] = static_cast<uint8_t>(255 *
      std::max(0.0f, std::min(1.0f, pixel.x())));
    data[3 * i + 1] = static_cast<uint8_t>(255 *
      std::max(0.0f, std::min(1.0f, pixel.y())));
    data[3 * i + 2] = static_cast<uint8_t>(255 *
      std::max(0.0f, std::min(1.0f, pixel.z())));
  }

  int status = stbi_write_png(filename, w, h, 3 /*RGB*/, data.data(),
                              3 * w * sizeof(uint8_t));
  return status == 1 ? true : false;
}

} // image

#endif // RTBENCH_IMAGE_H_