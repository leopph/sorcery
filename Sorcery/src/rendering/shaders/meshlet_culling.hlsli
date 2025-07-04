#ifndef MESHLET_CULLING_HLSLI
#define MESHLET_CULLING_HLSLI


struct MeshletCullData {
  float4 bounding_sphere;
  uint normal_cone;
  float apex_offset;
};


bool IsConeDegenerate(MeshletCullData const cull_data) {
  return (cull_data.normal_cone >> 24) == 0xff;
}


float4 UnpackCone(uint const packed) {
  float4 v;
  v.x = float((packed >> 0) & 0xFF);
  v.y = float((packed >> 8) & 0xFF);
  v.z = float((packed >> 16) & 0xFF);
  v.w = float((packed >> 24) & 0xFF);

  v = v / 255.0;
  v.xyz = v.xyz * 2.0 - 1.0;

  return v;
}


bool IsMeshletVisible(MeshletCullData c, float4x4 const local_to_world_mtx, float4 const frustum_planes_ws[6],
                      float const scale, float3 const view_pos) {
  // Do a cull test of the bounding sphere against the view frustum planes.
  float4 center = mul(float4(c.bounding_sphere.xyz, 1), local_to_world_mtx);
  float radius = c.bounding_sphere.w * scale;

  for (int i = 0; i < 6; ++i) {
    if (dot(center, frustum_planes_ws[i]) < -radius) {
      return false;
    }
  }

  // Do normal cone culling
  if (IsConeDegenerate(c)) {
    return true; // Cone is degenerate - spread is wider than a hemisphere.
  }

  // Unpack the normal cone from its 8-bit uint compression
  float4 const normal_cone = UnpackCone(c.normal_cone);

  // Transform axis to world space
  float3 axis = normalize(mul(float4(normal_cone.xyz, 0), local_to_world_mtx)).xyz;

  // Offset the normal cone axis from the meshlet center-point - make sure to account for world scaling
  float3 apex = center.xyz - axis * c.apex_offset * scale;
  float3 view = normalize(view_pos - apex);

  // The normal cone w-component stores -cos(angle + 90 deg)
  // This is the min dot product along the inverted axis from which all the meshlet's triangles are backface
  if (dot(view, -axis) > normal_cone.w) {
    return false;
  }

  // All tests passed - it will merit pixels
  return true;
}


#endif
