#include "CBuffers.hlsli"
#include "MeshVSOut.hlsli"

float4 main(const MeshVsOut vsOut) : SV_TARGET {
    const float3 ambient = 0.03 * material.albedo;
    float3 color = ambient;

    for (int i = 0; i < lightCount; i++)
    {
        switch (lights[i].type) {
            case 0: {
                    const float3 dirToLight = normalize(-lights[i].direction);
                    const float3 normal = normalize(vsOut.normal);
                    const float diffuse = saturate(dot(dirToLight, normal));

                    const float3 dirToCam = normalize(camPos - vsOut.worldPos);

                    const float3 halfway = normalize(dirToLight + dirToCam);
                    const float specular = pow(saturate(dot(normal, halfway)), 32);

                    color += material.albedo * lights[i].color * lights[i].intensity * (diffuse + specular);
                    break;
				}

            case 1: {
					break;
				}

            case 2: {
            		break;
				}

            default: {
		    		break;
				}
        }
    }

    return float4(color, 1);
}