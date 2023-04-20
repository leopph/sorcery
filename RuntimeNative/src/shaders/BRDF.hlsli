#ifndef BRDF_HLSLI
#define BRDF_HLSLI


static const float PI = 3.14159265359;


float DistributionTrowbridgeReitz(const float3 N, const float3 H, const float roughness) {
    const float a = pow(roughness, 2);
    const float a2 = pow(a, 2);
    const float NdotH = max(dot(N, H), 0.0);
    const float NdotH2 = pow(NdotH, 2);
	
    const float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * pow(denom, 2);
	
    return num / denom;
}


float GeometrySchlickTrowbridgeReitz(const float NdotV, const float roughness) {
    const float r = roughness + 1.0;
    const float k = pow(r, 2) / 8.0;

    const float num = NdotV;
    const float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}


float GeomertySmith(const float3 N, const float3 V, const float3 L, const float roughness) {
    const float NdotV = max(dot(N, V), 0.0);
    const float NdotL = max(dot(N, L), 0.0);
    const float ggx2 = GeometrySchlickTrowbridgeReitz(NdotV, roughness);
    const float ggx1 = GeometrySchlickTrowbridgeReitz(NdotL, roughness);
	
    return ggx1 * ggx2;
}


float3 FresnelSchlick(const float cosTheta, const float3 F0) {
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}


float3 CookTorrance(const float3 N, const float3 V, const float3 L, const float3 albedo, const float metallic, const float roughness, const float3 lightColor, const float lightIntensity, const float lightAtten) {
    float3 F0 = 0.04;
    F0 = lerp(F0, albedo, metallic);

    // calculate per-light radiance
    const float3 H = normalize(V + L);
    const float3 radiance = lightColor * lightIntensity * lightAtten;
        
    // cook-torrance brdf
    const float NDF = DistributionTrowbridgeReitz(N, H, roughness);
    const float G = GeomertySmith(N, V, L, roughness);
    const float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
        
    const float3 numerator = NDF * G * F;
    const float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    const float3 specular = numerator / denominator;

    const float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
            
    const float NdotL = max(dot(N, L), 0.0);

    // outgoing radiance
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

#endif