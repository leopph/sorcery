#include "include/CBuffers.hlsli"
#include "include/MeshVSOut.hlsli"

float4 main(const MeshVsOut vsOut) : SV_TARGET {
    const float3 ambient = 0.03 * material.albedo;
    float3 color = ambient;
	
    if (calcDirLight) {
        const float3 dirToLight = normalize(-dirLight.direction);
        const float3 normal = normalize(vsOut.normal);
        const float diffuse = saturate(dot(dirToLight, normal));

        const float3 dirToCam = normalize(camPos - vsOut.worldPos);
        
        const float3 halfway = normalize(dirToLight + dirToCam);
        const float specular = pow(saturate(dot(normal, halfway)), 32);

        color += material.albedo * dirLight.color * dirLight.intensity * (diffuse + specular);
    }

    const float3 correctedColor = pow(color, invGamma);
    return float4(correctedColor, 1);
}