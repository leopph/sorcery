#include "shader_interop.h"

struct DispatchParams {
  uint in_tex_idx;
  uint out_tex_idx;
  uint per_frame_cb_idx;
};

ConstantBuffer<DispatchParams> g_dispatch_params : register(b0, space0);
 
[numthreads(DEPTH_RESOLVE_CS_THREADS_X, DEPTH_RESOLVE_CS_THREADS_X, DEPTH_RESOLVE_CS_THREADS_Z)]
void main(const uint3 dtid : SV_DispatchThreadID) {
  uint2 tex_size;
  uint sample_count;

  const Texture2DMS<float> in_tex = ResourceDescriptorHeap[g_dispatch_params.in_tex_idx];
  in_tex.GetDimensions(tex_size.x, tex_size.y, sample_count);
   
  if (dtid.x > tex_size.x || dtid.y > tex_size.y) {
    return;
  }

  const ConstantBuffer<ShaderPerFrameConstants> per_frame_cb = ResourceDescriptorHeap[g_dispatch_params.per_frame_cb_idx];
  float result = 1 - per_frame_cb.isUsingReversedZ;

  const RWTexture2D<float> out_tex = ResourceDescriptorHeap[g_dispatch_params.out_tex_idx];

  if (per_frame_cb.isUsingReversedZ) {
    for (uint i = 0; i < sample_count; ++i) {
      result = max(result, in_tex.Load(dtid.xy, i).r);
    }
  } else {
    for (uint i = 0; i < sample_count; ++i) {
      result = min(result, in_tex.Load(dtid.xy, i).r);
    }
  }
   
  out_tex[dtid.xy] = result;
}
