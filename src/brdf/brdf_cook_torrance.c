// brdf_cook_torrance.c
#include <math.h>
#include <stdlib.h>
#include "algo.h"
#include "brdf/brdf.h"

typedef struct {
  wf_vec3 albedo;
  float   roughness; // roughness^2
  float   metallic;
} ct_data_t;

// Schlick Fresnel
static wf_vec3 fresnel_schlick(float cos_theta, const wf_vec3* F0) {
  float t = powf(1.0f - cos_theta, 5.0f);
  return (wf_vec3){ F0->x + (1.0f - F0->x) * t, F0->y + (1.0f - F0->y) * t,
                    F0->z + (1.0f - F0->z) * t };
}

// GGX 法线分布函数 NDF
static float ggx_ndf(float cos_h, float alpha) {
  if (cos_h <= 0.0f)
    return 0.0f;
  float alpha2 = alpha * alpha;
  float denom  = cos_h * cos_h * (alpha2 - 1.0f) + 1.0f;
  return alpha2 / (M_PI * denom * denom);
}

// Smith 几何遮蔽函数
static float smith_g1(float cos_theta, float alpha) {
  if (cos_theta <= 0.0f)
    return 0.0f;
  float alpha2 = alpha * alpha;
  float tan2   = (1.0f - cos_theta * cos_theta) / (cos_theta * cos_theta);
  return 2.0f / (1.0f + sqrtf(1.0f + alpha2 * tan2));
}

static float smith_g2(float cos_i, float cos_o, float alpha) {
  return smith_g1(cos_i, alpha) * smith_g1(cos_o, alpha);
}

// === Eval ===
static wf_vec3 ct_eval(const brdf_t* self, const wf_vec3* wi, const wf_vec3* wo,
                       const wf_vec3* n) {
  ct_data_t* data = (ct_data_t*)self->data;

  float ndotwi = v3_dot(*n, *wi);
  float ndotwo = v3_dot(*n, *wo);
  if (ndotwi <= 0.0f || ndotwo <= 0.0f) {
    return (wf_vec3){ 0, 0, 0 };
  }

  // 半角向量
  wf_vec3 h =
      v3_normalize((wf_vec3){ wi->x + wo->x, wi->y + wo->y, wi->z + wo->z });
  float ndoth = v3_dot(*n, h);
  float hdowi = v3_dot(h, *wi);

  // 基础反射率 F0
  wf_vec3 F0 = {
    0.04f * (1.0f - data->metallic) + data->albedo.x * data->metallic,
    0.04f * (1.0f - data->metallic) + data->albedo.y * data->metallic,
    0.04f * (1.0f - data->metallic) + data->albedo.z * data->metallic
  };

  // Fresnel
  wf_vec3 F = fresnel_schlick(hdowi, &F0);

  // NDF
  float alpha = data->roughness * data->roughness;
  float D     = ggx_ndf(ndoth, alpha);

  // Geometry term
  float G = smith_g2(ndotwi, ndotwo, alpha);

  // Cook-Torrance 公式
  float denominator = 4.0f * ndotwi * ndotwo;
  if (denominator < 1e-6f)
    return (wf_vec3){ 0, 0, 0 };

  wf_vec3 specular = { F.x * D * G / denominator, F.y * D * G / denominator,
                       F.z * D * G / denominator };

  // 漫反射（非金属部分）
  wf_vec3 kS = F; // 镜面反射比例
  wf_vec3 kD = { 1.0f - kS.x, 1.0f - kS.y, 1.0f - kS.z };
  // 金属度混合
  kD.x *= (1.0f - data->metallic);
  kD.y *= (1.0f - data->metallic);
  kD.z *= (1.0f - data->metallic);

  wf_vec3 diffuse = { kD.x * data->albedo.x / M_PI,
                      kD.y * data->albedo.y / M_PI,
                      kD.z * data->albedo.z / M_PI };

  return (wf_vec3){ diffuse.x + specular.x, diffuse.y + specular.y,
                    diffuse.z + specular.z };
}

// === Sample: GGX Importance Sampling ===
static wf_vec3 ct_sample(const brdf_t* self, const wf_vec3* wo,
                         const wf_vec3* n, float u1, float u2, wf_vec3* wi_out,
                         float* pdf_out) {
  ct_data_t* data = (ct_data_t*)self->data;

  // 采样微表面法线 m（GGX 分布）
  float phi = 2.0f * M_PI * u2;
  float cos_theta_m_sq =
      (1.0f - u1) / (u1 * (data->roughness * data->roughness - 1.0f) + 1.0f);
  float cos_theta_m = sqrtf(cos_theta_m_sq);
  float sin_theta_m = sqrtf(1.0f - cos_theta_m_sq);

  wf_vec3 m_local = { sin_theta_m * cosf(phi), sin_theta_m * sinf(phi),
                      cos_theta_m };

  // 转换到世界坐标（需构建 tangent frame）
  wf_vec3 up =
      fabsf(n->z) < 0.999f ? (wf_vec3){ 0, 0, 1 } : (wf_vec3){ 1, 0, 0 };
  wf_vec3 tangent   = v3_normalize((wf_vec3){ n->y * up.z - n->z * up.y,
                                              n->z * up.x - n->x * up.z,
                                              n->x * up.y - n->y * up.x });
  wf_vec3 bitangent = v3_normalize((wf_vec3){
      n->y * tangent.z - n->z * tangent.y, n->z * tangent.x - n->x * tangent.z,
      n->x * tangent.y - n->y * tangent.x });

  wf_vec3 m = {
    m_local.x * tangent.x + m_local.y * bitangent.x + m_local.z * n->x,
    m_local.x * tangent.y + m_local.y * bitangent.y + m_local.z * n->y,
    m_local.x * tangent.z + m_local.y * bitangent.z + m_local.z * n->z
  };

  // 反射方向 wi = reflect(-wo, m)
  float wodotm = v3_dot(*wo, m);
  wi_out->x    = 2.0f * wodotm * m.x - wo->x;
  wi_out->y    = 2.0f * wodotm * m.y - wo->y;
  wi_out->z    = 2.0f * wodotm * m.z - wo->z;

  if (v3_dot(*n, *wi_out) <= 0.0f) {
    *pdf_out = 0.0f;
    return (wf_vec3){ 0, 0, 0 };
  }

  // 计算 PDF（重要性采样下的 PDF）
  float denom = 4.0f * wodotm;
  if (denom < 1e-6f) {
    *pdf_out = 0.0f;
    return (wf_vec3){ 0, 0, 0 };
  }

  float alpha = data->roughness * data->roughness;
  float d     = ggx_ndf(cos_theta_m, alpha);
  float g     = smith_g1(v3_dot(*n, *wo), alpha);
  *pdf_out    = d * g * cos_theta_m / denom;

  // 返回 f_r(wi, wo)
  return ct_eval(self, wi_out, wo, n);
}

static void* ct_create(const void* params) {
  const struct {
    wf_vec3 albedo;
    float   roughness;
    float   metallic;
  }*         p    = params;
  ct_data_t* data = malloc(sizeof(ct_data_t));
  data->albedo    = p->albedo;
  data->roughness = p->roughness;
  data->metallic  = p->metallic;
  return data;
}

static void ct_destroy(void* data) {
  free(data);
}

static brdf_ops_t cook_torrance_ops = { .name    = "cook_torrance",
                                        .eval    = ct_eval,
                                        .sample  = ct_sample,
                                        .create  = ct_create,
                                        .destroy = ct_destroy };

__attribute__((constructor)) static void register_cook_torrance(void) {
  brdf_register_model(&cook_torrance_ops);
}
