#ifndef PBR_MATERIAL_HLSLI
#define PBR_MATERIAL_HLSLI

struct PbrMaterial {
    float3 albedo;
    float metallic;
    float roughness;
    float ao;
    int sampleAlbedo;
};

#endif