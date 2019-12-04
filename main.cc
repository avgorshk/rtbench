#include <assert.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>

#include "image.h"
#include "light.h"
#include "material.h"
#include "render.h"
#include "sphere.h"

int main(int argc, char* argv[]) {
  int w = 0, h = 0;
  std::vector<Vector> image;
  bool loaded = image::Load("input.jpg", w, h, image);
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

  auto start = std::chrono::steady_clock::now();
  render::Render(spheres, lights, image, w, h);
  auto end = std::chrono::steady_clock::now();
  auto elapsed =
    std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << "Wall Time: " << elapsed.count() << " ms (" <<
    std::fixed << std::setprecision(2) << 1000.0f /
    static_cast<float>(elapsed.count()) << " FPS)" << std::endl;

  bool saved = image::SavePng("out.png", w, h, image);
  assert(saved);
  return 0;
}