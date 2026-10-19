#ifndef TONEMAPPING_HLSLI
#define TONEMAPPING_HLSLI


float3 RrtAndOdtFit(float3 const color) {
  float3 a = color * (color + 0.0245786f) - 0.000090537f;
  float3 b = color * (0.983729f * color + 0.4329510f) + 0.238081f;
  return a / b;
}


float3 TonemapReinhard(float3 const color) {
  return color / (color + 1.0);
}


float3 TonemapAcesFilmic(float3 color) {
  static float3x3 const aces_input_matrix =
  {
    0.59719f, 0.07600f, 0.02840f,
    0.35458f, 0.90834f, 0.13383f,
    0.04823f, 0.01566f, 0.83777f
  };

  static float3x3 const aces_output_matrix =
  {
    1.60475f, -0.10208f, -0.00327f,
    -0.53108f, 1.10813f, -0.07276f,
    -0.07367f, -0.00605f, 1.07602f
  };

  color = mul(color, aces_input_matrix);
  color = RrtAndOdtFit(color);
  return mul(color, aces_output_matrix);
}


float3 AgxDefaultContrastApprox(float3 const x) {
  float3 x2 = x * x;
  float3 x4 = x2 * x2;
  return 15.5 * x4 * x2 - 40.14 * x4 * x + 31.96 * x4 - 6.868 * x2 * x + 0.4298 * x2 + 0.1191 * x - 0.00232;
}


float3 TonemapAgx(float3 color) {
  static float3x3 const agx_mtx =
  {
    0.842479062253094, 0.0423282422610123, 0.0423756549057051,
    0.0784335999999992, 0.878468636469772, 0.0784336,
    0.0792237451477643, 0.0791661274605434, 0.879142973793104
  };

  static float3x3 const agx_mtx_inv =
  {
    1.19687900512017, -0.0528968517574562, -0.0529716355144438,
    -0.0980208811401368, 1.15190312990417, -0.0980434501171241,
    -0.0990297440797205, -0.0989611768448433, 1.15107367264116
  };

  static float const min_ev = -12.47393;
  static float const max_ev = 4.026069;

  color = mul(color, agx_mtx);
  color = clamp(log2(max(color, 1e-10)), min_ev, max_ev);
  color = (color - min_ev) / (max_ev - min_ev);
  color = AgxDefaultContrastApprox(color);
  color = mul(color, agx_mtx_inv);
  return pow(max(color, 0), 2.2);
}

#endif
