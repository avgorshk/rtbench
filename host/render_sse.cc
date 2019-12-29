#include "render_sse.h"

#include <algorithm>
#include <vector>

#include <assert.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <immintrin.h>
#include <xmmintrin.h>

namespace host {
namespace sse {

inline __m128 Normalize(const __m128& v) {
  // *this = *this * (1.0f / norm());
  __m128 vval = _mm_dp_ps(v, v, 0xFF);
  vval = _mm_sqrt_ps(vval);
  vval = _mm_div_ps(_mm_set_ps1(1.0f), vval);
  return _mm_mul_ps(v, vval);
}

inline __m128 Reflect(const __m128& vi, const __m128& vn) {
  // return i - n * 2.0f * (i * n);
  __m128 vval = _mm_dp_ps(vi, vn, 0xFF);
  vval = _mm_mul_ps(vval, _mm_set_ps1(2.0f));
  vval = _mm_mul_ps(vval, vn);
  vval = _mm_sub_ps(vi, vval);
  return vval;
}

static __m128 Refract(const __m128& vi, const __m128& vn,
                      const float eta_t, const float eta_i = 1.f) {

  // float cosi = -std::max(-1.0f, std::min(1.0f, i * n));
  __m128 vcosi = _mm_dp_ps(vi, vn, 0xFF);
  float cosi = -std::max(-1.0f, std::min(1.0f, vcosi.m128_f32[0]));
  
  // if (cosi < 0) return Refract(i, -n, eta_i, eta_t);
  if (cosi < 0) {
    return Refract(vi, _mm_sub_ps(_mm_set_ps1(0.0f), vn), eta_i, eta_t);
  }

  float eta = eta_i / eta_t;
  float k = 1 - eta * eta * (1 - cosi * cosi);
  if (k < 0) {
    return _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);
  } else {
    // return i * eta + n * (eta * cosi - sqrtf(k));
    __m128 veta = _mm_set_ps1(eta);
    __m128 vres = _mm_mul_ps(vi, veta);
    float val = eta * cosi - sqrtf(k);
    __m128 vval = _mm_set_ps1(val);
    vval = _mm_mul_ps(vn, vval);
    vres = _mm_add_ps(vres, vval);
    return vres;
  }
}

static bool RayIntersect(const Sphere& sphere, const __m128& vorig,
                         const __m128& vdir, float& t0) {
  // Vector L = sphere.center() - orig;
  __m128 vc = _mm_load_ps(sphere.center().data());
  __m128 vL = _mm_sub_ps(vc, vorig);

  // float tca = L * dir;
  __m128 vtca = _mm_dp_ps(vL, vdir, 0xFF);

  // float d2 = L * L - tca * tca;
  __m128 vLdp = _mm_dp_ps(vL, vL, 0xFF);
  float d2 = vLdp.m128_f32[0] - vtca.m128_f32[0] * vtca.m128_f32[0];

  if (d2 > sphere.radius()* sphere.radius()) {
    return false;
  }

  float thc = sqrtf(sphere.radius() * sphere.radius() - d2);
  t0 = vtca.m128_f32[0] - thc;
  float t1 = vtca.m128_f32[0] + thc;
  if (t0 < 0) {
    t0 = t1;
  }
  if (t0 < 0) {
    return false;
  }
  return true;
}

static bool SceneIntersect(const __m128& vorig, const __m128& vdir,
                           const std::vector<Sphere>& spheres,
                           __m128& vhit, __m128& vnorm,
                           Material& material) {
  float spheres_dist = std::numeric_limits<float>::max();
  for (size_t i = 0; i < spheres.size(); i++) {
    float dist_i = 0.0f;
    if (RayIntersect(spheres[i], vorig, vdir, dist_i) &&
        dist_i < spheres_dist) {
      spheres_dist = dist_i;
      
      // hit = orig + dir * dist_i;
      vhit = _mm_add_ps(vorig, _mm_mul_ps(vdir, _mm_set_ps1(dist_i)));

      //norm = (hit - spheres[i].center()).Normalize();
      vnorm = Normalize(
        _mm_sub_ps(vhit, _mm_load_ps(spheres[i].center().data())));

      material = spheres[i].material();
    }
  }

  float checkerboard_dist = std::numeric_limits<float>::max();
  if (fabsf(vdir.m128_f32[1]) > 1e-3f) {
    float d = -(vorig.m128_f32[1] + 4.0f) / vdir.m128_f32[1];
    
    // Vector pt = orig + dir * d;
    __m128 vpt = _mm_add_ps(vorig, _mm_mul_ps(vdir, _mm_set_ps1(d)));
    
    if (d > 0 && fabs(vpt.m128_f32[0]) < 10.0f &&
        vpt.m128_f32[2] < -10.0f && vpt.m128_f32[2] > -30.0f &&
        d < spheres_dist) {
      checkerboard_dist = d;
      //hit = pt;
      vhit = vpt;
      vnorm = _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f);
      material.SetDiffuseColor(
        (static_cast<int>(0.5f * vhit.m128_f32[0] + 1000.0f) +
        (static_cast<int>(0.5f * vhit.m128_f32[2])) & 1) ?
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

  __m128 vorig = _mm_load_ps(orig.data());
  __m128 vdir = _mm_load_ps(dir.data());
  __m128 vpoint = _mm_set_ps1(0.0f);
  __m128 vnorm = _mm_set_ps1(0.0f);

  if (depth > 4 || !SceneIntersect(vorig, vdir, spheres,
                                   vpoint, vnorm, material)) {
    return background;
  }

  _mm_store_ps(point.data(), vpoint);
  _mm_store_ps(norm.data(), vnorm);

  __m128 vreflect_dir = Normalize(Reflect(vdir, vnorm));
  Vector reflect_dir;
  _mm_store_ps(reflect_dir.data(), vreflect_dir);

  __m128 vrefract_dir = Normalize(Refract(vdir, vnorm, material.refractive_index()));
  Vector refract_dir;
  _mm_store_ps(refract_dir.data(), vrefract_dir);


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

    __m128 vshadow_orig = _mm_load_ps(shadow_orig.data());
    __m128 vlight_dir = _mm_load_ps(light_dir.data());
    __m128 vshadow_pt = _mm_set_ps1(0.0f);
    __m128 vshadow_n = _mm_set_ps1(0.0f);

    bool intersected = SceneIntersect(vshadow_orig, vlight_dir, spheres,
                                      vshadow_pt, vshadow_n, tmpmaterial);

    _mm_store_ps(shadow_pt.data(), vshadow_pt);
    _mm_store_ps(shadow_n.data(), vshadow_n);

    if (intersected && ((shadow_pt - shadow_orig).norm() < light_distance)) {
      continue;
    }

    diffuse_light_intensity += lights[i].intensity() *
      std::max(0.f, light_dir * norm);

    Vector mlight_dir = -light_dir;
    __m128 vmlight_dir = _mm_load_ps(mlight_dir.data());
    __m128 vreflect = Reflect(vmlight_dir, vnorm);
    Vector reflect;
    _mm_store_ps(reflect.data(), vreflect);

    specular_light_intensity +=
      powf(std::max(0.0f, -reflect * dir),
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

  //#pragma omp parallel for
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

} // namespace sse
} // namespace host