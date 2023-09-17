#include "ShaderInterop.h"

TEXTURE2DMS(gInput, float, RES_SLOT_DEPTH_RESOLVE_INPUT);
RWTEXTURE2D(gOutput, float, UAV_SLOT_DEPTH_RESOLVE_OUTPUT);
 
[numthreads(DEPTH_RESOLVE_CS_THREADS_X, DEPTH_RESOLVE_CS_THREADS_X, DEPTH_RESOLVE_CS_THREADS_Z)]
void main(const uint3 dispatchThreadId : SV_DispatchThreadID) {
  uint2 texSize;
  uint sampleCount;
  gInput.GetDimensions(texSize.x, texSize.y, sampleCount);
   
  if (dispatchThreadId.x > texSize.x || dispatchThreadId.y > texSize.y) {
    return;
  }
   
  float result = 1 - gPerFrameConstants.isUsingReversedZ;

  if (gPerFrameConstants.isUsingReversedZ) {
    for (uint i = 0; i < sampleCount; ++i) {
      result = max(result, gInput.Load(dispatchThreadId.xy, i).r);
    }
  } else {
    for (uint i = 0; i < sampleCount; ++i) {
      result = min(result, gInput.Load(dispatchThreadId.xy, i).r);
    }
  }
   
  gOutput[dispatchThreadId.xy] = result;
}
