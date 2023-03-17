#ifndef PBR_MATERIAL_HLSLI
#define PBR_MATERIAL_HLSLI

struct PbrMaterial {
    float3 albedo;
    float metallic;
    float roughness;
    float ao;
    int sampleAlbedo;
    int sampleMetallic;
    int sampleRoughness;
    int sampleAo;
};

Texture2D gAlbedoMap : register(t0);
Texture2D gMetallicMap : register(t1);
Texture2D gRoughnessMap : register(t2);
Texture2D gAoMap : register(t3);

SamplerState gMaterialSampler : register(s0);

#endif