#include "MeshVSOut.hlsli"
#include "ShaderInterop.h"
#include "BRDF.hlsli"


TEXTURE2D(gAlbedoMap, float4, TEX_SLOT_ALBEDO_MAP);
TEXTURE2D(gMetallicMap, float, TEX_SLOT_METALLIC_MAP);
TEXTURE2D(gRoughnessMap, float, TEX_SLOT_ROUGHNESS_MAP);
TEXTURE2D(gAoMap, float, TEX_SLOT_AO_MAP);
TEXTURE2D(gPunctualShadowAtlas, float, TEX_SLOT_PUNCTUAL_SHADOW_ATLAS);

SAMPLERSTATE(gMaterialSampler, SAMPLER_SLOT_MATERIAL);
SAMPLERCOMPARISONSTATE(gShadowSampler, SAMPLER_SLOT_SHADOW);

STRUCTUREDBUFFER(lights, ShaderLight, SB_SLOT_LIGHTS);


inline float2 TransformUVForShadowAtlas(const float2 uv, const uint atlasQuadrantIdx, const uint atlasCellIdx) {
    const int cellRowColCount = pow(2, atlasQuadrantIdx);
    float cellSize = 0.5f / cellRowColCount;

    float2 quadrantOffset = float2(0, 0);

    if (atlasQuadrantIdx == 1) {
        quadrantOffset = float2(0.5f, 0);
    }
    else if (atlasQuadrantIdx == 2) {
        quadrantOffset = float2(0, 0.5f);
    }
    else if (atlasQuadrantIdx == 3) {
        quadrantOffset = float2(0.5f, 0.5f);
    }

    const float2 cellOffset = float2(atlasCellIdx % cellRowColCount, atlasCellIdx / cellRowColCount) * float2(cellSize, cellSize);

    return uv * cellSize + quadrantOffset + cellOffset;
}


inline float SampleShadowCascadeFromAtlas(const Texture2D<float> atlas, const float3 fragWorldPos, const uint lightIdx, const uint cascadeIdx) {
    const float4 posLClip = mul(float4(fragWorldPos, 1), lights[lightIdx].shadowViewProjMatrices[cascadeIdx]);
    float3 posLNdc = posLClip.xyz / posLClip.w;
    posLNdc.xy = posLNdc.xy * float2(0.5, -0.5) + 0.5;

    [branch]
    if (lights[lightIdx].atlasQuadrantIndices[cascadeIdx] == INVALID_IDX) {
        return 1.0f;
    }

    return atlas.SampleCmpLevelZero(gShadowSampler, TransformUVForShadowAtlas(posLNdc.xy, lights[lightIdx].atlasQuadrantIndices[cascadeIdx], lights[lightIdx].atlasCellIndices[cascadeIdx]), posLNdc.z);
}


inline float CalculateAttenuation(const float distance) {
    return 1 / pow(distance, 2);
}


inline float3 CalculateDirLight(const float3 N, const float3 V, const float3 albedo, const float metallic, const float roughness, const int lightIdx) {
    const float3 L = -lights[lightIdx].direction;
    return CookTorrance(N, V, L, albedo, metallic, roughness, lights[lightIdx].color, lights[lightIdx].intensity, 1);
}


inline float3 CalculateSpotLight(const float3 N, const float3 V, const float3 albedo, const float metallic, const float roughness, const int lightIdx, const float3 fragWorldPos)
{
	float3 L = lights[lightIdx].position - fragWorldPos;
    const float dist = length(L);
    L = normalize(L);

    const float rangeMul = float(dist <= lights[lightIdx].range);
    const float thetaCos = dot(L, -lights[lightIdx].direction);
    const float eps = lights[lightIdx].innerAngleCos - lights[lightIdx].outerAngleCos;
    const float intensity = saturate((thetaCos - lights[lightIdx].outerAngleCos) / eps);

    float3 lighting = CookTorrance(N, V, L, albedo, metallic, roughness, lights[lightIdx].color, lights[lightIdx].intensity, CalculateAttenuation(dist));
    lighting *= intensity;
    lighting *= rangeMul;

    [branch]
    if (lights[lightIdx].isCastingShadow) {
        lighting *= SampleShadowCascadeFromAtlas(gPunctualShadowAtlas, fragWorldPos, 0, 0);
    }
    
    return lighting;
}


inline float3 CalculatePointLight(const float3 N, const float3 V, const float3 albedo, const float metallic, const float roughness, const int lightIdx, const float3 fragWorldPos)
{
	float3 L = lights[lightIdx].position - fragWorldPos;
    const float dist = length(L);
    L = normalize(L);

    const float rangeMul = float(dist <= lights[lightIdx].range);

    float3 lighting = CookTorrance(N, V, L, albedo, metallic, roughness, lights[lightIdx].color, lights[lightIdx].intensity, CalculateAttenuation(dist));
    lighting *= rangeMul;

    [branch]
    if (lights[lightIdx].isCastingShadow) {
        const float3 dirToFrag = fragWorldPos - lights[lightIdx].position;

        uint maxIdx = abs(dirToFrag.x) > abs(dirToFrag.y) ? 0 : 1;
        maxIdx = abs(dirToFrag[maxIdx]) > abs(dirToFrag.z) ? maxIdx : 2;
        uint cascadeIdx = maxIdx * 2;

        if (sign(dirToFrag[maxIdx]) < 0) {
            cascadeIdx += 1;
        }

        lighting *= SampleShadowCascadeFromAtlas(gPunctualShadowAtlas, fragWorldPos, lightIdx, cascadeIdx);
    }

    return lighting;

}


float4 main(const MeshVsOut vsOut) : SV_TARGET {
    const float3 N = normalize(vsOut.normal);
    const float3 V = normalize(camPos - vsOut.worldPos);

    float3 albedo = material.albedo;

    if (material.sampleAlbedo) {
        albedo *= pow(gAlbedoMap.Sample(gMaterialSampler, vsOut.uv).rgb, 2.2);
    }

    float metallic = material.metallic;

    if (material.sampleMetallic) {
        metallic *= gMetallicMap.Sample(gMaterialSampler, vsOut.uv).r;
    }

    float roughness = material.roughness;

    if (material.sampleRoughness) {
        roughness *= gRoughnessMap.Sample(gMaterialSampler, vsOut.uv).r;
    }

    float ao = material.ao;

    if (material.sampleAo) {
        ao *= gAoMap.Sample(gMaterialSampler, vsOut.uv).r;
    }

    float3 outColor = 0.03 * albedo * ao;

    for (int i = 0; i < lightCount && i < MAX_LIGHT_COUNT; i++)
    {
        switch (lights[i].type)
        {
            case 0:{
                    outColor += CalculateDirLight(N, V, albedo, metallic, roughness, i);
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