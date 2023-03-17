#include "CBuffers.hlsli"
#include "MeshVSOut.hlsli"

Texture2D<float> gSpotPointShadowAtlas : register(t4);
SamplerComparisonState gShadowSampler : register(s1);

float SampleShadowMap(const Texture2D<float> shadowMap, const Light light, const float3 posW) {
    const float4 posLClip = mul(float4(posW, 1), light.lightSpaceMtx);
    float3 posLNdc = posLClip.xyz / posLClip.w;
    posLNdc.xy = posLNdc.xy * 0.5 + 0.5;
    return shadowMap.SampleCmpLevelZero(gShadowSampler, posLNdc.xy * light.shadowAtlasScale + light.shadowAtlasOffset, posLNdc.z);
}

static const float PI = 3.14159265f;

float3 FresnelSchlick(const float cosTheta, const float3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float TrowbridgeReitz(const float3 N, const float3 H, const float roughness) {
    const float a = roughness * roughness;
    const float a2 = a * a;
    const float NdotH = max(dot(N, H), 0.0);
    const float NdotH2 = NdotH * NdotH;
	
    const float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float SchlickTrowbridgeReitz(const float NdotV, const float roughness) {
    const float r = (roughness + 1.0);
    const float k = (r * r) / 8.0;

    const float num = NdotV;
    const float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float Smith(const float3 N, const float3 V, const float3 L, const float roughness) {
    const float NdotV = max(dot(N, V), 0.0);
    const float NdotL = max(dot(N, L), 0.0);
    const float ggx2 = SchlickTrowbridgeReitz(NdotV, roughness);
    const float ggx1 = SchlickTrowbridgeReitz(NdotL, roughness);
	
    return ggx1 * ggx2;
}

float3 CalculateLighting(const float3 N, const float3 V, const float3 L, const float3 albedo, const float metallic, const float roughness, const float3 lightColor, const float lightIntensity) {
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);
	           
    // reflectance equation
    float3 Lo = float3(0.0, 0.0, 0.0);
        // calculate per-light radiance
    const float3 H = normalize(V + L);
    const float3 radiance = lightColor * lightIntensity;
        
        // cook-torrance brdf
    const float NDF = TrowbridgeReitz(N, H, roughness);
    const float G = Smith(N, V, L, roughness);
    const float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
        
    const float3 kS = F;
    float3 kD = float3(1.0, 1.0, 1.0) - kS;
    kD *= 1.0 - metallic;
        
    const float3 numerator = NDF * G * F;
    const float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    const float3 specular = numerator / denominator;
            
        // add to outgoing radiance Lo
    const float NdotL = max(dot(N, L), 0.0);
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
   
    return Lo;
}

float CalculateAttenuation(const float distance) {
    return 1 / pow(distance, 2);
}

float3 CalculateDirLight(const float3 N, const float3 V, const float3 albedo, const float metallic, const float roughness, const Light light) {
	const float3 L = -light.direction;
    return CalculateLighting(N, V, L, albedo, metallic, roughness, light.color, light.intensity);
}

float3 CalculateSpotLight(const float3 N, const float3 V, const float3 albedo, const float metallic, const float roughness, const Light light, const float3 fragWorldPos)
{
	float3 L = light.position - fragWorldPos;
    const float dist = length(L);

    if (dist > light.range) {
	    return float3(0, 0, 0);
    }

    L = normalize(L);

    const float thetaCos = dot(L, -light.direction);
    const float eps = light.innerAngleCos - light.outerAngleCos;
    const float intensity = saturate((thetaCos - light.outerAngleCos) / eps);

    if (intensity == 0) {
	    return float3(0, 0, 0);
    }

    float3 lighting = CalculateLighting(N, V, L, albedo, metallic, roughness, light.color, light.intensity);
    lighting *= intensity;
    lighting *= CalculateAttenuation(dist);

    if (light.isCastingShadow) {
        lighting *= SampleShadowMap(gSpotPointShadowAtlas, light, fragWorldPos);
    }
    
    return lighting;
}

float3 CalculatePointLight(const float3 N, const float3 V, const float3 albedo, const float metallic, const float roughness, const Light light, const float3 fragWorldPos)
{
	float3 L = light.position - fragWorldPos;
    const float dist = length(L);

    if (dist > light.range) {
	    return float3(0, 0, 0);
    }

    L = normalize(L);
    return CalculateLighting(N, V, L, albedo, metallic, roughness, light.color, light.intensity) * CalculateAttenuation(dist);
}

float4 main(const MeshVsOut vsOut) : SV_TARGET {
    const float3 N = normalize(vsOut.normal);
    const float3 V = normalize(camPos - vsOut.worldPos);

    float3 albedo = material.albedo;

    if (material.sampleAlbedo != 0) {
        albedo *= pow(gAlbedoMap.Sample(gMaterialSampler, vsOut.uv).rgb, 2.2);
    }

    float metallic = material.metallic;

    if (material.sampleMetallic != 0) {
        metallic *= gMetallicMap.Sample(gMaterialSampler, vsOut.uv).r;
    }

    float roughness = material.roughness;

    if (material.sampleRoughness != 0) {
        roughness *= gRoughnessMap.Sample(gMaterialSampler, vsOut.uv).r;
    }

    float ao = material.ao;

    if (material.sampleAo != 0) {
        ao *= gAoMap.Sample(gMaterialSampler, vsOut.uv).r;
    }

    float3 outColor = 0.03 * albedo * ao;

    for (int i = 0; i < lightCount; i++)
    {
        switch (lights[i].type)
        {
            case 0:{
                    outColor += CalculateDirLight(N, V, albedo, metallic, roughness, lights[i]);
                    break;
                }
            case 1:{
                    outColor += CalculateSpotLight(N, V, albedo, metallic, roughness, lights[i], vsOut.worldPos);
                    break;
                }
            case 2:{
                    outColor += CalculatePointLight(N, V, albedo, metallic, roughness, lights[i], vsOut.worldPos);
                    break;
                }
            default:{
                    break;
                }
        }
    }

    return float4(outColor, 1);
}