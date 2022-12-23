#include "include/Lights.hlsli"

cbuffer Material : register(b0) {
    float3 albedo;
    float metallic;
    float roughness;
    float ao;
};

cbuffer CameraStuff : register(b1) {
    float3 camPos;
};

cbuffer Lights : register(b2) {
    DirectionalLight dirLight;
    bool calcDirLight;
}

static const float PI = 3.14159265f;

float3 FresnelSchlick(float cosTheta, float3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float TrowbridgeReitz(float3 N, float3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
	
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float SchlickTrowbridgeReitz(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float Smith(float3 N, float3 V, float3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = SchlickTrowbridgeReitz(NdotV, roughness);
    float ggx1 = SchlickTrowbridgeReitz(NdotL, roughness);
	
    return ggx1 * ggx2;
}

struct PSIn {
    float3 worldPos : worldPos;
    float3 normal : NORMAL;
};

float4 main(PSIn input) : SV_TARGET {
    float3 N = normalize(input.normal);
    float3 V = normalize(camPos - input.worldPos);

    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);
	           
    // reflectance equation
    float3 Lo = float3(0.0, 0.0, 0.0);
        // calculate per-light radiance
    float3 L = normalize(-dirLight.direction);
    float3 H = normalize(V + L);
    float3 radiance = dirLight.color * dirLight.intensity;
        
        // cook-torrance brdf
    float NDF = TrowbridgeReitz(N, H, roughness);
    float G = Smith(N, V, L, roughness);
    float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
        
    float3 kS = F;
    float3 kD = float3(1.0, 1.0, 1.0) - kS;
    kD *= 1.0 - metallic;
        
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    float3 specular = numerator / denominator;
            
        // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
  
    float3 ambient = 0.03 * albedo * ao;
    float3 color = ambient + Lo;
	
    color = color / (color + 1.0);
    color = pow(color, 1.0 / 2.2);
   
    return float4(color, 1.0);
}