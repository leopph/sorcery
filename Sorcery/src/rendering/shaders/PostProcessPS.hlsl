struct DrawParams {
  uint in_tex_idx;
  float inv_gamma;
};

ConstantBuffer<DrawParams> g_draw_params : register(b0, space0);


float4 main(const float4 pixel_coord : SV_POSITION) : SV_TARGET {
  const Texture2D in_tex = ResourceDescriptorHeap[g_draw_params.in_tex_idx];
  float3 pixel_color = in_tex.Load(int3(pixel_coord.xy, 0)).rgb;

  // Tone mapping
  pixel_color = pixel_color / (pixel_color + 1.0);

  // Gamma correction
  pixel_color = pow(pixel_color, g_draw_params.inv_gamma);

  return float4(pixel_color, 1);
}
