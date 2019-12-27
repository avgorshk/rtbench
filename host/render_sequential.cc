#include "render_sequential.h"

#include <algorithm>
#include <vector>

#include <assert.h>
#define _USE_MATH_DEFINES
#include <math.h>

namespace host {
namespace sequential {

static Vector Reflect(const Vector& i, const Vector& n) {
  return i - n * 2.0f * (i * n);
}

static Vector Refract(const Vector& i, const Vector& n,
                      const float eta_t, const float eta_i = 1.f) {
  float cosi = -std::max(-1.0f, std::min(1.0f, i * n));
  if (cosi < 0) return Refract(i, -n, eta_i, eta_t);
  float eta = eta_i / eta_t;
  float k = 1 - eta * eta * (1 - cosi * cosi);
  return k < 0 ? Vector(1.0f, 0.0f, 0.0f) :
    i * eta + n * (eta * cosi - sqrtf(k));
}

static bool RayIntersect(const Sphere& sphere, const Vector& orig,
                         const Vector& dir, float& t0) {
  Vector L = sphere.center() - orig;
  float tca = L * dir;
  float d2 = L * L - tca * tca;
  if (d2 > sphere.radius()* sphere.radius()) {
    return false;
  }
  float thc = sqrtf(sphere.radius() * sphere.radius() - d2);
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

static bool SceneIntersect(const Vector& orig, const Vector& dir,
                           const std::vector<Sphere>& spheres,
                           Vector& hit, Vector& norm,
                           Material& material) {
  float spheres_dist = std::numeric_limits<float>::max();
  for (size_t i = 0; i < spheres.size(); i++) {
    float dist_i = 0.0f;
    if (RayIntersect(spheres[i], orig, dir, dist_i) && dist_i < spheres_dist) {
      spheres_dist = dist_i;
      hit = orig + dir * dist_i;
      norm = (hit - spheres[i].center()).Normalize();
      material = spheres[i].material();
    }
  }

  float checkerboard_dist = std::numeric_limits<float>::max();
  if (fabsf(dir.y()) > 1e-3f) {
    float d = -(orig.y() + 4.0f) / dir.y();
    Vector pt = orig + dir * d;
    if (d > 0 && fabs(pt.x()) < 10.0f &&
        pt.z() < -10.0f && pt.z() > -30.0f &&
        d < spheres_dist) {
      checkerboard_dist = d;
      hit = pt;
      norm = Vector(0.0f, 1.0f, 0.0f);
      material.SetDiffuseColor(
        (static_cast<int>(0.5f * hit.x() + 1000.0f) +
        (static_cast<int>(0.5f * hit.z())) & 1) ?
        Vector(0.3f, 0.3f, 0.3f) : Vector(0.3f, 0.2f, 0.1f));
    }
  }
  return std::min(spheres_dist, checkerboard_dist) < 1000.0f;
}

static Vector CastRay(const Vector& background,
               const Vector& orig, const Vector& dir,
               const std::vector<Sphere>& spheres,
               const std::vector<Light>& lights,
               size_t depth = 0) {
  Vector point, norm;
  Material material;

  if (depth > 4 || !SceneIntersect(orig, dir, spheres,
                                   point, norm, material)) {
    return background;
  }

  Vector reflect_dir = Reflect(dir, norm).Normalize();
  Vector refract_dir = Refract(dir, norm,
                               material.refractive_index()).Normalize();
  Vector reflect_orig = reflect_dir * norm < 0 ?
    point - norm * 1e-3f : point + norm * 1e-3f;
  Vector refract_orig = refract_dir * norm < 0 ?
    point - norm * 1e-3f : point + norm * 1e-3f;
  Vector reflect_color = CastRay(background, reflect_orig, reflect_dir,
                                 spheres, lights, depth + 1);
  Vector refract_color = CastRay(background, refract_orig, refract_dir,
                                 spheres, lights, depth + 1);

  float diffuse_light_intensity = 0, specular_light_intensity = 0;
  for (size_t i = 0; i < lights.size(); i++) {
    Vector light_dir = (lights[i].position() - point).Normalize();
    float light_distance = (lights[i].position() - point).norm();

    Vector shadow_orig = light_dir * norm < 0 ?
      point - norm * 1e-3f : point + norm * 1e-3f;
    Vector shadow_pt, shadow_n;
    Material tmpmaterial;
    if (SceneIntersect(shadow_orig, light_dir, spheres,
                       shadow_pt, shadow_n, tmpmaterial) &&
                       (shadow_pt - shadow_orig).norm() < light_distance) {
      continue;
    }

    diffuse_light_intensity += lights[i].intensity() *
      std::max(0.f, light_dir * norm);
    specular_light_intensity +=
      powf(std::max(0.0f, -Reflect(-light_dir, norm) * dir),
           material.specular_exponent()) * lights[i].intensity();
  }

  return material.diffuse_color() * diffuse_light_intensity *
    material.albedo().x() + Vector(1., 1., 1.) * specular_light_intensity *
    material.albedo().y() + reflect_color * material.albedo().z() +
    refract_color * material.albedo().w();
}


void Render(const std::vector<Sphere>& spheres,
            const std::vector<Light>& lights,
            std::vector<Vector>& image,
            int w, int h) {
  assert(image.size() == w * h);
  const float fov = static_cast<float>(M_PI / 3.0);

  for (int i = 0; i < h; ++i) {
    for (int j = 0; j < w; ++j) {
      float dir_x = (j + 0.5f) - w / 2.0f;
      float dir_y = -(i + 0.5f) + h / 2.0f;
      float dir_z = -h / (2.0f * tanf(fov / 2.0f));
      image[i * w + j] = CastRay(image[i * w + j],
                                 Vector(0.0f, 0.0f, 0.0f),
                                 Vector(dir_x, dir_y, dir_z).Normalize(),
                                 spheres,
                                 lights);
    }
  }
}

} // namespace sequential
} // namespace host