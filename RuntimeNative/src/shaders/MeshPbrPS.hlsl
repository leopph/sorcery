#include "MeshVSOut.hlsli"
#include "ShaderInterop.h"
#include "BRDF.hlsli"
#include "Samplers.hlsli"
#include "Util.hlsli"


TEXTURE2D(gAlbedoMap, float4, RES_SLOT_ALBEDO_MAP);
TEXTURE2D(gMetallicMap, float, RES_SLOT_METALLIC_MAP);
TEXTURE2D(gRoughnessMap, float, RES_SLOT_ROUGHNESS_MAP);
TEXTURE2D(gAoMap, float, RES_SLOT_AO_MAP);
TEXTURE2D(gPunctualShadowAtlas, float, RES_SLOT_PUNCTUAL_SHADOW_ATLAS);
TEXTURE2D(gDirShadowAtlas, float, RES_SLOT_DIR_SHADOW_ATLAS);

STRUCTUREDBUFFER(gLights, ShaderLight, RES_SLOT_LIGHTS);


inline float SampleShadowCascadeFromAtlas(const Texture2D<float> atlas, const float3 fragWorldPos, const uint lightIdx, const uint shadowMapIdx, const float3 fragNormal) {
    uint atlasSize;
    atlas.GetDimensions(atlasSize, atlasSize);
    const float atlasTexelSize = 1.0 / atlasSize;
	const float shadowMapTexelSize = atlasTexelSize / gLights[lightIdx].shadowUvScales[shadowMapIdx];

    const float4 posLClip = mul(float4(fragWorldPos + fragNormal * shadowMapTexelSize * gLights[lightIdx].normalBias, 1), gLights[lightIdx].shadowViewProjMatrices[shadowMapIdx]);
    float3 posLNdc = posLClip.xyz / posLClip.w;
    posLNdc.xy = posLNdc.xy * float2(0.5, -0.5) + 0.5;

	return atlas.SampleCmpLevelZero(gSamplerCmpPoint, posLNdc.xy * gLights[lightIdx].shadowUvScales[shadowMapIdx] + gLights[lightIdx].shadowUvOffsets[shadowMapIdx], posLNdc.z + shadowMapTexelSize * gLights[lightIdx].depthBias);
}


inline float CalculateAttenuation(const float distance) {
    return 1 / pow(distance, 2);
}


inline float3 CalculateDirLight(const float3 N, const float3 V, const float3 albedo, const float metallic, const float roughness, const int lightIdx, const float3 fragPosWorld, const float fragPosViewZ) {
    const float3 L = -gLights[lightIdx].direction;

    float3 lighting;

    [branch]
    if (gPerFrameConstants.visualizeShadowCascades) {
        lighting = VisualizeShadowCascades(fragPosViewZ);
    }
    else {
        lighting = CookTorrance(N, V, L, albedo, metallic, roughness, gLights[lightIdx].color, gLights[lightIdx].intensity, 1);
    }

    [branch]
    if (gLights[lightIdx].isCastingShadow) {
        int cascadeIdx = 0;

        while (cascadeIdx < gPerFrameConstants.shadowCascadeCount && fragPosViewZ > gPerCamConstants.shadowCascadeFarBounds[cascadeIdx]) {
            cascadeIdx += 1;
        }

        [branch]
        if (cascadeIdx != gPerFrameConstants.shadowCascadeCount) {
	        lighting *= SampleShadowCascadeFromAtlas(gDirShadowAtlas, fragPosWorld, lightIdx, cascadeIdx, N);
        }
    }

    return lighting;
}


inline float3 CalculateSpotLight(const float3 N, const float3 V, const float3 albedo, const float metallic, const float roughness, const int lightIdx, const float3 fragWorldPos)
{
	float3 L = gLights[lightIdx].position - fragWorldPos;
    const float dist = length(L);
    L = normalize(L);

    const float rangeMul = float(dist <= gLights[lightIdx].range);
    const float thetaCos = dot(L, -gLights[lightIdx].direction);
    const float eps = gLights[lightIdx].innerAngleCos - gLights[lightIdx].outerAngleCos;
    const float intensity = saturate((thetaCos - gLights[lightIdx].outerAngleCos) / eps);

    float3 lighting = CookTorrance(N, V, L, albedo, metallic, roughness, gLights[lightIdx].color, gLights[lightIdx].intensity, CalculateAttenuation(dist));
    lighting *= intensity;
    lighting *= rangeMul;

    [branch]
    if (gLights[lightIdx].isCastingShadow) {
        lighting *= SampleShadowCascadeFromAtlas(gPunctualShadowAtlas, fragWorldPos, lightIdx, 0, N);
    }
    
    return lighting;
}


inline float3 CalculatePointLight(const float3 N, const float3 V, const float3 albedo, const float metallic, const float roughness, const int lightIdx, const float3 fragWorldPos)
{
	float3 L = gLights[lightIdx].position - fragWorldPos;
    const float dist = length(L);
    L = normalize(L);

    const float rangeMul = float(dist <= gLights[lightIdx].range);

    float3 lighting = CookTorrance(N, V, L, albedo, metallic, roughness, gLights[lightIdx].color, gLights[lightIdx].intensity, CalculateAttenuation(dist));
    lighting *= rangeMul;

    [branch]
    if (gLights[lightIdx].isCastingShadow) {
        const float3 dirToFrag = fragWorldPos - gLights[lightIdx].position;

        uint maxIdx = abs(dirToFrag.x) > abs(dirToFrag.y) ? 0 : 1;
        maxIdx = abs(dirToFrag[maxIdx]) > abs(dirToFrag.z) ? maxIdx : 2;
        uint shadowMapIdx = maxIdx * 2;

        if (sign(dirToFrag[maxIdx]) < 0) {
            shadowMapIdx += 1;
        }

        [branch]
        if (gLights[lightIdx].sampleShadowMap[shadowMapIdx]) {
            lighting *= SampleShadowCascadeFromAtlas(gPunctualShadowAtlas, fragWorldPos, lightIdx, shadowMapIdx, N);
        }
    }

    return lighting;
}


float4 main(const MeshVsOut vsOut) : SV_TARGET {
    const float3 N = normalize(vsOut.normal);
    const float3 V = normalize(gPerCamConstants.camPos - vsOut.worldPos);

    float3 albedo = material.albedo;

    if (material.sampleAlbedo) {
        albedo *= pow(gAlbedoMap.Sample(gSamplerAf16, vsOut.uv).rgb, 2.2);
    }

    float metallic = material.metallic;

    if (material.sampleMetallic) {
        metallic *= gMetallicMap.Sample(gSamplerAf16, vsOut.uv).r;
    }

    float roughness = material.roughness;

    if (material.sampleRoughness) {
        roughness *= gRoughnessMap.Sample(gSamplerAf16, vsOut.uv).r;
    }

    float ao = material.ao;

    if (material.sampleAo) {
        ao *= gAoMap.Sample(gSamplerAf16, vsOut.uv).r;
    }

    float3 outColor = 0.03 * albedo * ao;

    uint lightCount;
    uint _;
    gLights.GetDimensions(lightCount, _);

    for (uint i = 0; i < lightCount; i++)
    {
        switch (gLights[i].type)
        {
            case 0:{
                    outColor += CalculateDirLight(N, V, albedo, metallic, roughness, i, vsOut.worldPos, vsOut.viewPosZ);
                    break;
                }
            case 1:{
                    outColor += CalculateSpotLight(N, V, albedo, metallic, roughness, i, vsOut.worldPos);
                    break;
                }
            case 2:{
                    outColor += CalculatePointLight(N, V, albedo, metallic, roughness, i, vsOut.worldPos);
                    break;
                }
            default:{
                    break;
                }
        }
    }

    return float4(outColor, 1);
}