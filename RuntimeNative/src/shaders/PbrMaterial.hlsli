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

Texture2D gAlbedoMap;
Texture2D gMetallicMap;
Texture2D gRoughnessMap;
Texture2D gAoMap;
SamplerState gMaterialSampler;

#endif