#include "render_sse.h"

#include <algorithm>
#include <vector>

#include <assert.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <immintrin.h>
#include <xmmintrin.h>

namespace sse {

inline __m128 Normalize(const __m128& v) {
  __m128 vval = _mm_dp_ps(v, v, 0xFF);
  vval = _mm_sqrt_ps(vval);
  vval = _mm_div_ps(_mm_set_ps1(1.0f), vval);
  return _mm_mul_ps(v, vval);
}

inline __m128 Reflect(const __m128& vi, const __m128& vn) {
  __m128 vval = _mm_dp_ps(vi, vn, 0xFF);
  vval = _mm_mul_ps(vval, _mm_set_ps1(2.0f));
  vval = _mm_mul_ps(vval, vn);
  vval = _mm_sub_ps(vi, vval);
  return vval;
}

inline __m128 Refract(const __m128& vi, const __m128& vn,
                      const float eta_t, const float eta_i = 1.f) {
  __m128 vcosi = _mm_dp_ps(vi, vn, 0xFF);
  float cosi = -std::max(-1.0f, std::min(1.0f, vcosi.m128_f32[0]));
  if (cosi < 0) {
    return Refract(vi, _mm_sub_ps(_mm_set_ps1(0.0f), vn), eta_i, eta_t);
  }

  float eta = eta_i / eta_t;
  float k = 1 - eta * eta * (1 - cosi * cosi);
  if (k < 0) {
    return _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);
  } else {
    __m128 vres = _mm_mul_ps(vi, _mm_set_ps1(eta));
    float val = eta * cosi - sqrtf(k);
     __m128 vval = _mm_mul_ps(vn, _mm_set_ps1(val));
    vres = _mm_add_ps(vres, vval);
    return vres;
  }
}

inline bool RayIntersect(const Sphere& sphere, const __m128& vorig,
                         const __m128& vdir, float& t0) {
  __m128 vc = _mm_load_ps(sphere.center().data());
  __m128 vL = _mm_sub_ps(vc, vorig);
  __m128 vtca = _mm_dp_ps(vL, vdir, 0xFF);
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

inline bool SceneIntersect(const __m128& vorig, const __m128& vdir,
                           const std::vector<Sphere>& spheres,
                           __m128& vhit, __m128& vnorm,
                           Material& material) {
  float spheres_dist = std::numeric_limits<float>::max();
  for (size_t i = 0; i < spheres.size(); i++) {
    float dist_i = 0.0f;
    if (RayIntersect(spheres[i], vorig, vdir, dist_i) &&
        dist_i < spheres_dist) {
      spheres_dist = dist_i;
      vhit = _mm_add_ps(vorig, _mm_mul_ps(vdir, _mm_set_ps1(dist_i)));
      vnorm = Normalize(
        _mm_sub_ps(vhit, _mm_load_ps(spheres[i].center().data())));
      material = spheres[i].material();
    }
  }

  float checkerboard_dist = std::numeric_limits<float>::max();
  if (fabsf(vdir.m128_f32[1]) > 1e-3f) {
    float d = -(vorig.m128_f32[1] + 4.0f) / vdir.m128_f32[1];
    __m128 vpt = _mm_add_ps(vorig, _mm_mul_ps(vdir, _mm_set_ps1(d)));
    
    if (d > 0 && fabs(vpt.m128_f32[0]) < 10.0f &&
        vpt.m128_f32[2] < -10.0f && vpt.m128_f32[2] > -30.0f &&
        d < spheres_dist) {
      checkerboard_dist = d;
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

__m128 CastRay(const __m128& vbackground,
               const __m128& vorig, const __m128& vdir,
               const std::vector<Sphere>& spheres,
               const std::vector<Light>& lights,
               size_t depth = 0) {
  Material material;
  __m128 vpoint = _mm_set_ps1(0.0f);
  __m128 vnorm = _mm_set_ps1(0.0f);

  if (depth > 4 || !SceneIntersect(vorig, vdir, spheres,
                                   vpoint, vnorm, material)) {
    return vbackground;
  }

  __m128 vreflect_dir = Normalize(Reflect(vdir, vnorm));
  __m128 vrefract_dir = Normalize(Refract(vdir, vnorm,
                                          material.refractive_index()));

  __m128 vreflect_orig = _mm_dp_ps(vreflect_dir, vnorm, 0xFF);
  if (vreflect_orig.m128_f32[0] < 0) {
    vreflect_orig = _mm_sub_ps(vpoint, _mm_mul_ps(vnorm, _mm_set_ps1(1e-3f)));
  } else {
    vreflect_orig = _mm_add_ps(vpoint, _mm_mul_ps(vnorm, _mm_set_ps1(1e-3f)));
  }

  __m128 vrefract_orig = _mm_dp_ps(vrefract_dir, vnorm, 0xFF);
  if (vrefract_orig.m128_f32[0] < 0) {
    vrefract_orig = _mm_sub_ps(vpoint, _mm_mul_ps(vnorm, _mm_set_ps1(1e-3f)));
  } else {
    vrefract_orig = _mm_add_ps(vpoint, _mm_mul_ps(vnorm, _mm_set_ps1(1e-3f)));
  }

  __m128 vreflect_color = CastRay(vbackground, vreflect_orig, vreflect_dir,
                                 spheres, lights, depth + 1);
  __m128 vrefract_color = CastRay(vbackground, vrefract_orig, vrefract_dir,
                                 spheres, lights, depth + 1);

  float diffuse_light_intensity = 0, specular_light_intensity = 0;
  for (size_t i = 0; i < lights.size(); i++) {
    __m128 vpos = _mm_load_ps(lights[i].position().data());
    __m128 vlight_dir = Normalize(_mm_sub_ps(vpos, vpoint));
    __m128 vlight_distance = _mm_sub_ps(vpos, vpoint);
    vlight_distance = _mm_dp_ps(vlight_distance, vlight_distance, 0xFF);
    vlight_distance = _mm_sqrt_ps(vlight_distance);

    __m128 vshadow_orig = _mm_dp_ps(vlight_dir, vnorm, 0xFF);
    if (vshadow_orig.m128_f32[0] < 0) {
      vshadow_orig = _mm_sub_ps(vpoint, _mm_mul_ps(vnorm, _mm_set_ps1(1e-3f)));
    } else {
      vshadow_orig = _mm_add_ps(vpoint, _mm_mul_ps(vnorm, _mm_set_ps1(1e-3f)));
    }

    Material tmpmaterial;
    __m128 vshadow_pt = _mm_set_ps1(0.0f);
    __m128 vshadow_n = _mm_set_ps1(0.0f);
    bool intersected = SceneIntersect(vshadow_orig, vlight_dir, spheres,
                                      vshadow_pt, vshadow_n, tmpmaterial);

    __m128 vval = _mm_sub_ps(vshadow_pt, vshadow_orig);
    vval = _mm_dp_ps(vval, vval, 0xFF);
    vval = _mm_sqrt_ps(vval);

    if (intersected && (vval.m128_f32[0] < vlight_distance.m128_f32[0])) {
      continue;
    }

    vval = _mm_dp_ps(vlight_dir, vnorm, 0xFF);
    diffuse_light_intensity += lights[i].intensity() *
      std::max(0.0f, vval.m128_f32[0]);

    __m128 vmlight_dir = _mm_sub_ps(_mm_set_ps1(0.0f), vlight_dir);
    __m128 vreflect = Reflect(vmlight_dir, vnorm);
    vreflect = _mm_sub_ps(_mm_set_ps1(0.0f), vreflect);
    vreflect = _mm_dp_ps(vreflect, vdir, 0xFF);

    specular_light_intensity +=
      powf(std::max(0.0f, vreflect.m128_f32[0]),
           material.specular_exponent()) * lights[i].intensity();
  }

  __m128 vdiffuse_color = _mm_load_ps(material.diffuse_color().data());
  __m128 vres = _mm_mul_ps(vdiffuse_color, _mm_set_ps1(diffuse_light_intensity * material.albedo().x()));
  vres = _mm_add_ps(vres, _mm_mul_ps(_mm_set_ps(0.0f, 1.0f, 1.0f, 1.0f), _mm_set_ps1(specular_light_intensity * material.albedo().y())));
  vres = _mm_add_ps(vres, _mm_mul_ps(vreflect_color, _mm_set_ps1(material.albedo().z())));
  vres = _mm_add_ps(vres, _mm_mul_ps(vrefract_color, _mm_set_ps1(material.albedo().w())));
  return vres;
}


void Render(const std::vector<Sphere>& spheres,
            const std::vector<Light>& lights,
            std::vector<Vector>& image,
            int w, int h) {
  assert(image.size() == w * h);
  const float kFov = static_cast<float>(M_PI / 3.0);

  #pragma omp parallel for
  for (int i = 0; i < h; ++i) {
    for (int j = 0; j < w; ++j) {
      __m128 vdir = _mm_set_ps(0.0f, -h / (2.0f * tanf(kFov / 2.0f)),
                               -(i + 0.5f) + h / 2.0f, (j + 0.5f) - w / 2.0f);
      __m128 vpixel = _mm_load_ps(image[i * w + j].data());
      vpixel = CastRay(vpixel,
                       _mm_set_ps1(0.0f),
                       Normalize(vdir),
                       spheres,
                       lights);
      _mm_store_ps(image[i * w + j].data(), vpixel);
    }
  }
}

} // namespace sse