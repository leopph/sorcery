#ifndef TONEMAPPING_HLSLI
#define TONEMAPPING_HLSLI

static float3x3 const aces_input_matrix = {
  0.59719f, 0.07600f, 0.02840f,
  0.35458f, 0.90834f, 0.13383f,
  0.04823f, 0.01566f, 0.83777f
};

static float3x3 const aces_output_matrix = {
  1.60475f, -0.10208f, -0.00327f,
  -0.53108f, 1.10813f, -0.07276f,
  -0.07367f, -0.00605f, 1.07602f
};


float3 RrtAndOdtFit(float3 const color) {
  float3 a = color * (color + 0.0245786f) - 0.000090537f;
  float3 b = color * (0.983729f * color + 0.4329510f) + 0.238081f;
  return a / b;
}


float3 TonemapReinhard(float3 const color) {
  return color / (color + 1.0);
}


float3 TonemapAcesFilmic(float3 color) {
  color = mul(color, aces_input_matrix);
  color = RrtAndOdtFit(color);
  return mul(color, aces_output_matrix);
}

#endif
