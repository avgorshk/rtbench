#include <assert.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>

#include "device.h"
#include "image.h"
#include "light.h"
#include "material.h"
#include "sphere.h"

const unsigned kFrameCount = 10;

static void Usage() {
  std::cout << "How To Tun: rtbech -d [device] -v [version]" << std::endl;
  std::cout << "Available Devices:" << std::endl;
  std::vector<std::string> deviceList = device::GetDeviceList();
  assert(deviceList.size() > 0);
  for (size_t i = 0; i < deviceList.size(); ++i) {
    std::cout << "  [" << i << "] " <<  deviceList[i] << std::endl;
    std::cout << "  Available Versions:" << std::endl;
    std::vector<std::string> versionList =
      device::GetVersionList(static_cast<int>(i));
    assert(versionList.size() > 0);
    for (size_t j = 0; j < versionList.size(); ++j) {
      std::cout << "    [" << j << "] " << versionList[j] << std::endl;
    }
  }
}

int main(int argc, char* argv[]) {
  int device = -1, version = -1;
  for (int i = 1; i < argc - 1; ++i) {
    if (strcmp(argv[i], "-d") == 0) {
      device = atoi(argv[i + 1]);
    } else if (strcmp(argv[i], "-v") == 0) {
      version = atoi(argv[i + 1]);
    }
  }

  if (device < 0 || version < 0) {
    Usage();
    return 0;
  }

  std::vector<std::string> deviceList = device::GetDeviceList();
  assert(deviceList.size() > 0);
  std::vector<std::string> versionList = device::GetVersionList(device);
  if (versionList.size() == 0) {
    std::cout << "Invalid device " << device << std::endl;
    Usage();
    return 0;
  }

  if (version >= versionList.size()) {
    std::cout << "Invalid version " << version << " for device " <<
      device << std::endl;
    Usage();
    return 0;
  }

  std::cout << "Target Device: " << deviceList[device] << std::endl;
  std::cout << "Target Version: " << versionList[version] << std::endl;

  int w = 0, h = 0;
  std::vector<Vector> input;
  bool loaded = image::Load("input.jpg", w, h, input);
  if (!loaded) {
    std::cout << "Input image file was not found" << std::endl;
    return 0;
  }

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

  std::vector<Sphere> spheres;
  spheres.push_back(Sphere(Vector(-3.0f, 0.0f, -16.0f), 2.0f, ivory));
  spheres.push_back(Sphere(Vector(-1.0f, -1.5f, -12.0f), 2.0f, glass));
  spheres.push_back(Sphere(Vector(1.5f, -0.5f, -18.0f), 3.0f, red_rubber));
  spheres.push_back(Sphere(Vector(7.0f, 5.0f, -18.0f), 4.0f, mirror));

  std::vector<Light> lights;
  lights.push_back(Light(Vector(-20.0f, 20.0f, 20.0f), 1.5f));
  lights.push_back(Light(Vector(30.0f, 50.0f, -25.0f), 1.8f));
  lights.push_back(Light(Vector(30.0f, 20.0f, 30.0f), 1.7f));

  std::vector<Vector> output(input.size(), { 0.0f });

  std::cout << "Warming-up...";
  std::copy(input.begin(), input.end(), output.begin());
  bool succeed = device::Render(spheres, lights, output, w, h, device, version);
  assert(succeed);
  std::cout << "DONE" << std::endl;

  std::cout << "Computing...";
  unsigned wall_time = 0;
  for (unsigned i = 0; i < kFrameCount; ++i) {
    std::copy(input.begin(), input.end(), output.begin());
    auto start = std::chrono::steady_clock::now();
    bool succeed = device::Render(spheres, lights, output, w, h, device, version);
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
  bool same = image::Compare(output, "reference.png");
  if (!same) {
    std::cout << "FAIL" << std::endl;
  } else {
    std::cout << "OK" << std::endl;
  }

  bool saved = image::SavePng("output.png", w, h, output);
  assert(saved);

  return 0;
}