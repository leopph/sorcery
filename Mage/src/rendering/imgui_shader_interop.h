#ifndef IMGUI_SHADER_INTEROP_H
#define IMGUI_SHADER_INTEROP_H

#ifdef __cplusplus
#include "Math.hpp"
#include <cstdint>
using float4x4 = sorcery::Matrix4;
using float4 = sorcery::Vector4;
using float2 = sorcery::Vector2;
using uint = std::uint32_t;
#endif

struct VertexData {
  float2 pos_os;
  float2 uv;
  float4 col;
};

struct ImGuiDrawParams {
  float4x4 proj_mtx;
  uint vb_idx;
  uint tex_idx;
  uint samp_idx;
};

#endif
