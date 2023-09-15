#include "ShaderInterop.h"
#include "Samplers.hlsli"
#include "ScreenVsOut.hlsli"
#include "Util.hlsli"

TEXTURE2D(gDepthTexture, float, RES_SLOT_SSAO_DEPTH);
TEXTURE2D(gNormalTexture, float3, RES_SLOT_SSAO_NORMAL);
TEXTURE2D(gNoiseTexture, float4, RES_SLOT_SSAO_NOISE);

STRUCTUREDBUFFER(gSamples, float4, RES_SLOT_SSAO_SAMPLES);

float3 UvToPositionVS(const float2 uv) {
  const float depth = gDepthTexture.Sample(gSamplerPoint, uv).r;
	const float4 positionVS = mul(float4(UvToNdc(uv), depth, 1), gPerViewConstants.invProjMtx);
	return positionVS.xyz / positionVS.w;
}

float main(const ScreenVsOut vsOut) : SV_TARGET {
	float2 noiseTexSize;
	gNoiseTexture.GetDimensions(noiseTexSize.x, noiseTexSize.y);
	const float2 noiseScale = gPerFrameConstants.screenSize / noiseTexSize;

	const float3 randomVec = normalize(gNoiseTexture.Sample(gSamplerPoint, vsOut.uv * noiseScale).xyz);
	const float3 positionVS = UvToPositionVS(vsOut.uv);
	const float3 normalVS = normalize(mul(float4(gNormalTexture.Sample(gSamplerPoint, vsOut.uv).xyz, 0), gPerViewConstants.viewMtx).xyz);
	const float3 tangentVS = normalize(randomVec - normalVS * dot(randomVec, normalVS));
	const float3 bitangentVS = cross(normalVS, tangentVS);
	const float3x3 tbnMtxVS = float3x3(tangentVS, bitangentVS, normalVS);

	float occlusion = 0.0;

	uint kernelSize, stride;
	gSamples.GetDimensions(kernelSize, stride);

	for (uint i = 0; i < kernelSize; i++) {
	  const float3 samplePos = mul(gSamples[i].xyz, tbnMtxVS) * gSsaoConstants.radius + positionVS;

		float4 sampleOffset = mul(float4(samplePos, 1), gPerViewConstants.projMtx);
		sampleOffset.xy /= sampleOffset.w;
		sampleOffset.xy = NdcToUv(sampleOffset.xy);

		const float sampleDepth = UvToPositionVS(sampleOffset.xy).z;

		const float rangeCheck = smoothstep(0.0, 1.0, gSsaoConstants.radius / abs(positionVS.z - sampleDepth));
		occlusion += step(sampleDepth, samplePos.z + gSsaoConstants.bias) * rangeCheck;
	}

  return 1 - occlusion / kernelSize;
}
