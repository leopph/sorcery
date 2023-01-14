#include "include/CBuffers.hlsli"
#include "include/MeshVSOut.hlsli"

float4 main(MeshVsOut psIn) : SV_TARGET {
    float4 ret = float4(0.05, 0.05, 0.05, 1);
	
    if (calcDirLight) {
        float3 dirToLight = normalize(-dirLight.direction);
        float diffuse = saturate(dot(dirToLight, normalize(psIn.normal)));
        ret.xyz += diffuse * dirLight.color * dirLight.intensity;
    }

    return ret;
}