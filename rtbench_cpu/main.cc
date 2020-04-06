#include <assert.h>
#include <intrin.h>
#include <memory.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>

#include "common/image.h"
#include "common/scene.h"
#include "render_baseline.h"
#include "render_sequential.h"
#include "render_sse.h"

const unsigned kFrameCount = 10;

static inline std::string GetHostCPU() {
  int cpu_info[4] = { 0 };
  __cpuid(cpu_info, 0x80000000);

  unsigned n_ex_ids = cpu_info[0];
  char cpu_band_string[0x40] = { 0 };
  for (unsigned i = 0x80000000; i <= n_ex_ids; ++i) {
    __cpuid(cpu_info, i);
    if (i == 0x80000002) {
      memcpy(cpu_band_string, cpu_info, sizeof(cpu_info));
    } else if (i == 0x80000003) {
      memcpy(cpu_band_string + 16, cpu_info, sizeof(cpu_info));
    } else if (i == 0x80000004) {
      memcpy(cpu_band_string + 32, cpu_info, sizeof(cpu_info));
    }
  }

  return cpu_band_string;
}

inline std::vector<std::string> GetVersionList() {
  return std::vector<std::string>{"Sequential", "Baseline", "SSE"};
}

inline bool Render(const std::vector<Sphere>& spheres,
                   const std::vector<Light>& lights,
                   std::vector<Vector>& image,
                   int w, int h, int version) {
  if (version == 0) {
    sequential::Render(spheres, lights, image, w, h);
    return true;
  } else if (version == 1) {
    baseline::Render(spheres, lights, image, w, h);
    return true;
  } else if (version == 2) {
    sse::Render(spheres, lights, image, w, h);
    return true;
  }
  return false;
}

static void Usage() {
  std::cout << "How To Tun: rtbech -v <version> [-i <input.jpg>]" <<
    " [-r <reference.png>] [-o <output.png>]" << std::endl;
  std::cout << "Available Versions:" << std::endl;
  std::vector<std::string> version_list = GetVersionList();
  assert(version_list.size() > 0);
  for (size_t j = 0; j < version_list.size(); ++j) {
    std::cout << "[" << j << "] " << version_list[j] << std::endl;
  }
}

int main(int argc, char* argv[]) {
  int version = -1;
  std::string input_image("input.jpg");
  std::string output_image("output.png");
  std::string reference_image("reference.png");

  for (int i = 1; i < argc - 1; ++i) {
    if (strcmp(argv[i], "-v") == 0) {
      version = atoi(argv[i + 1]);
    } else if (strcmp(argv[i], "-i") == 0) {
      input_image = argv[i + 1];
    } else if (strcmp(argv[i], "-o") == 0) {
      output_image = argv[i + 1];
    } else if (strcmp(argv[i], "-r") == 0) {
      reference_image = argv[i + 1];
    }
  }

  if (version < 0) {
    Usage();
    return 0;
  }

  std::vector<std::string> version_list = GetVersionList();
  assert(version_list.size() > 0);

  if (version >= version_list.size()) {
    std::cout << "Invalid version " << version << std::endl;
    Usage();
    return 0;
  }

  std::cout << "Target Device: " << GetHostCPU() << std::endl;
  std::cout << "Target Version: " << version_list[version] << std::endl;

  int w = 0, h = 0;
  std::vector<Vector> input;
  bool loaded = image::Load(input_image.c_str(), w, h, input);
  if (!loaded) {
    std::cout << "Input image file was not found: " <<
      input_image.c_str() << std::endl;
    return 0;
  }

  std::cout << "Warming-up...";
  Scene scene(input);
  bool succeed = Render(scene.GetSpheres(), scene.GetLights(),
                        scene.GetImage(), w, h, version);
  assert(succeed);
  std::cout << "DONE" << std::endl;

  std::cout << "Computing...";
  unsigned wall_time = 0;
  for (unsigned i = 0; i < kFrameCount; ++i) {
    Scene scene(input);
    auto start = std::chrono::steady_clock::now();
    bool succeed = Render(scene.GetSpheres(), scene.GetLights(),
                          scene.GetImage(), w, h, version);
    assert(succeed);
    auto end = std::chrono::steady_clock::now();
    auto elapsed =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    wall_time += static_cast<unsigned>(elapsed.count());
    std::cout << ".";
  }
  std::cout << std::endl;
  
  std::cout << "Wall Time: " << wall_time << " ms" << std::endl;
  std::cout << "Time per Frame: " << wall_time / kFrameCount <<
    " ms" << std::endl;
  std::cout << "FPS rate: " << std::fixed << std::setprecision(2) <<
    kFrameCount * 1000.0f / wall_time << std::endl;

  std::cout << "Checking for results...";
  bool same = image::Compare(scene.GetImage(), reference_image.c_str());
  if (!same) {
    std::cout << "FAIL" << std::endl;
  } else {
    std::cout << "OK" << std::endl;
  }

  bool saved = image::SavePng(output_image.c_str(), w, h, scene.GetImage());
  assert(saved);

  return 0;
}