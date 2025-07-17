#ifndef BRDF_INTEGRATION_HLSLI
#define BRDF_INTEGRATION_HLSLI

#include "brdf.hlsli"
#include "fullscreen_tri.hlsli"


float2 PsMain(PsIn const ps_in) : SV_Target {
  return IntegrateBRDF(ps_in.uv.x, ps_in.uv.y);
}


#endif
