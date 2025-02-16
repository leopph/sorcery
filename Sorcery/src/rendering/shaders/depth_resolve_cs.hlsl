#include "common.hlsli"
#include "shader_interop.h"


DECLARE_PARAMS(DepthResolveDrawParams);


[numthreads(DEPTH_RESOLVE_CS_THREADS_X, DEPTH_RESOLVE_CS_THREADS_X, DEPTH_RESOLVE_CS_THREADS_Z)]
void main(const uint3 dtid : SV_DispatchThreadID) {
  uint2 tex_size;
  uint sample_count;

  const Texture2DMS<float> in_tex = ResourceDescriptorHeap[g_params.in_tex_idx];
  in_tex.GetDimensions(tex_size.x, tex_size.y, sample_count);

  if (dtid.x > tex_size.x || dtid.y > tex_size.y) {
    return;
  }

  float result = 0;

  const RWTexture2D<float> out_tex = ResourceDescriptorHeap[g_params.out_tex_idx];

  for (uint i = 0; i < sample_count; ++i) {
    result = max(result, in_tex.Load(dtid.xy, (int) i).r);
  }

  out_tex[dtid.xy] = result;
}
