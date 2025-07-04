#ifndef MESH_SHADER_CORE_HLSLI
#define MESH_SHADER_CORE_HLSLI

#include "meshlet_culling.hlsli"
#include "shader_interop.h"


uint3 UnpackTriangleIndices(uint const packed_indices) {
  return uint3(packed_indices & 0x3FF, (packed_indices >> 10) & 0x3FF, (packed_indices >> 20) & 0x3FF);
}


struct Meshlet {
  uint vertex_count;
  uint vertex_offset;
  uint primitive_count;
  uint primitive_offset;
};


#if !defined(MESH_SHADER_NO_PAYLOAD)
struct CullingPayload {
  uint meshlet_indices[AS_GROUP_SIZE];
};


groupshared CullingPayload g_payload;


void AmpShaderCore(
  uint const dtid,
  uint const meshlet_count,
  uint const cull_data_buf_idx,
  uint const per_draw_cb_idx,
  uint const per_view_cb_idx) {
  bool visible = false;

  StructuredBuffer<MeshletCullData> const cull_data = ResourceDescriptorHeap[cull_data_buf_idx];
  ConstantBuffer<ShaderPerDrawConstants> const per_draw_cb = ResourceDescriptorHeap[per_draw_cb_idx];
  ConstantBuffer<ShaderPerViewConstants> const per_view_cb = ResourceDescriptorHeap[per_view_cb_idx];

  // Check bounds of meshlet cull data resource
  if (dtid < meshlet_count) {
    // Do visibility testing for this thread
    visible = IsMeshletVisible(cull_data[dtid], per_draw_cb.modelMtx, per_view_cb.frustum_planes_ws,
      per_draw_cb.max_abs_scaling, per_view_cb.viewPos);
  }

  // Compact visible meshlets into the export payload array
  if (visible) {
    uint index = WavePrefixCountBits(visible);
    g_payload.meshlet_indices[index] = dtid;
  }

  // Dispatch the required number of MS threadgroups to render the visible meshlets
  uint const visible_count = WaveActiveCountBits(visible);
  DispatchMesh(visible_count, 1, 1, g_payload);
}
#endif


#define MS_THREAD_GROUP_SIZE 128
#define MS_VERTEX_LOOP_COUNT (MESHLET_MAX_VERTS + MS_THREAD_GROUP_SIZE - 1) / MS_THREAD_GROUP_SIZE
#define MS_PRIMITIVE_LOOP_COUNT (MESHLET_MAX_PRIMS + MS_THREAD_GROUP_SIZE - 1) / MS_THREAD_GROUP_SIZE
#define MS_VERTEX_PRIMITIVE_LOOP_STRIDE MS_THREAD_GROUP_SIZE

#ifdef MESH_SHADER_NO_PRIMITIVE_ATTRIBUTES
#define MESH_SHADER_BODY_PROCESS_PRIMITIVE
#else
#define MESH_SHADER_BODY_PROCESS_PRIMITIVE out_primitives[primitive_id] = PrimitiveProcessor::CalculatePrimitive(primitive_id);
#endif

#ifdef MESH_SHADER_NO_PAYLOAD
#define MESH_SHADER_BODY_GET_MESHLET_INDEX(gid, dispatch_instance_count, payload) gid / dispatch_instance_count
#else
#define MESH_SHADER_BODY_GET_MESHLET_INDEX(gid, dispatch_instance_count, payload) payload.meshlet_indices[gid]
#endif


#define MESH_SHADER_BODY(\
  gid,\
  gtid,\
  meshlet_buf_idx,\
  vertex_idx_buf_idx,\
  prim_idx_buf_idx,\
  dispatch_meshlet_offset,\
  dispatch_meshlet_count,\
  dispatch_instance_offset,\
  dispatch_instance_count,\
  base_vertex,\
  idx32,\
  payload,\
  out_vertices,\
  out_primitives,\
  out_indices)\
uint const meshlet_idx = MESH_SHADER_BODY_GET_MESHLET_INDEX(gid, dispatch_instance_count, payload);\
StructuredBuffer<Meshlet> const meshlets = ResourceDescriptorHeap[meshlet_buf_idx];\
Meshlet const meshlet = meshlets[meshlet_idx + dispatch_meshlet_offset];\
\
uint start_instance = gid % dispatch_instance_count;\
uint instance_count = 1;\
\
if (meshlet_idx == dispatch_meshlet_count - 1) {\
  uint const instances_per_group = min(MESHLET_MAX_VERTS / meshlet.vertex_count,\
    MESHLET_MAX_PRIMS / meshlet.primitive_count);\
    \
  uint const unpacked_group_count = (dispatch_meshlet_count - 1) * dispatch_instance_count;\
  uint const packed_index = gid - unpacked_group_count;\
  \
  start_instance = packed_index * instances_per_group;\
  instance_count = min(dispatch_instance_count - start_instance, instances_per_group);\
}\
\
uint const vert_count = meshlet.vertex_count * instance_count;\
uint const prim_count = meshlet.primitive_count * instance_count;\
\
SetMeshOutputCounts(vert_count, prim_count);\
\
for (uint i = 0; i < MS_VERTEX_LOOP_COUNT; i++) {\
  uint const vertex_id = gtid + i * MS_VERTEX_PRIMITIVE_LOOP_STRIDE;\
  \
  if (vertex_id < vert_count) {\
    uint const read_index = vertex_id % meshlet.vertex_count;\
    uint const instance_id = vertex_id / meshlet.vertex_count;\
    \
    ByteAddressBuffer const vertex_indices = ResourceDescriptorHeap[vertex_idx_buf_idx];\
    uint const vertex_index_idx = meshlet.vertex_offset + read_index;\
    \
    uint vertex_index;\
    \
    if (idx32) {\
      vertex_index = vertex_indices.Load(vertex_index_idx * 4);\
    } else {\
      uint const word_offset = vertex_index_idx & 0x1;\
      uint const byte_offset = (vertex_index_idx / 2) * 4;\
      \
      uint const index_pair = vertex_indices.Load(byte_offset);\
      vertex_index = (index_pair >> (word_offset * 16)) & 0xFFFF;\
    }\
    \
    vertex_index += base_vertex;\
    \
    uint const instance_index = dispatch_instance_offset + start_instance + instance_id;\
    \
    out_vertices[vertex_id] = VertexProcessor::CalculateVertex(vertex_index, instance_index);\
  }\
}\
\
for (uint i = 0; i < MS_PRIMITIVE_LOOP_COUNT; i++) {\
  uint const primitive_id = gtid + i * MS_VERTEX_PRIMITIVE_LOOP_STRIDE;\
  \
  if (primitive_id < prim_count) {\
    uint const read_index = primitive_id % meshlet.primitive_count;\
    uint const instance_id = primitive_id / meshlet.primitive_count;\
    \
    StructuredBuffer<uint> const primitive_indices = ResourceDescriptorHeap[prim_idx_buf_idx];\
    \
    out_indices[primitive_id] = UnpackTriangleIndices(primitive_indices[meshlet.primitive_offset + read_index])\
                                + (meshlet.vertex_count * instance_id);\
                                \
    MESH_SHADER_BODY_PROCESS_PRIMITIVE\
  }\
}


#ifdef MESH_SHADER_NO_PAYLOAD
#ifdef MESH_SHADER_NO_PRIMITIVE_ATTRIBUTES
#define MESH_SHADER_CORE(\
  gid,\
  gtid,\
  meshlet_buf_idx,\
  vertex_idx_buf_idx,\
  prim_idx_buf_idx,\
  dispatch_meshlet_offset,\
  dispatch_meshlet_count,\
  dispatch_instance_offset,\
  dispatch_instance_count,\
  base_vertex,\
  idx32,\
  out_vertices,\
  out_indices)\
MESH_SHADER_BODY(gid, gtid, meshlet_buf_idx, vertex_idx_buf_idx, prim_idx_buf_idx, dispatch_meshlet_offset,\
  dispatch_meshlet_count, dispatch_instance_offset, dispatch_instance_count, base_vertex, idx32, placeholder1,\
  out_vertices, placeholder2, out_indices)
#else
#define MESH_SHADER_CORE(\
  gid,\
  gtid,\
  meshlet_buf_idx,\
  vertex_idx_buf_idx,\
  prim_idx_buf_idx,\
  dispatch_meshlet_offset,\
  dispatch_meshlet_count,\
  dispatch_instance_offset,\
  dispatch_instance_count,\
  base_vertex,\
  idx32,\
  out_vertices,\
  out_primitives,\
  out_indices)\
MESH_SHADER_BODY(gid, gtid, meshlet_buf_idx, vertex_idx_buf_idx, prim_idx_buf_idx, dispatch_meshlet_offset,\
  dispatch_meshlet_count, dispatch_instance_offset, dispatch_instance_count, base_vertex, idx32, placeholder1,\
  out_vertices, out_primitives, out_indices)
#endif
#else
#ifdef MESH_SHADER_NO_PRIMITIVE_ATTRIBUTES
#define MESH_SHADER_CORE(\
  gid,\
  gtid,\
  meshlet_buf_idx,\
  vertex_idx_buf_idx,\
  prim_idx_buf_idx,\
  dispatch_meshlet_offset,\
  dispatch_meshlet_count,\
  dispatch_instance_offset,\
  dispatch_instance_count,\
  base_vertex,\
  idx32,\
  payload,\
  out_vertices,\
  out_indices)\
MESH_SHADER_BODY(gid, gtid, meshlet_buf_idx, vertex_idx_buf_idx, prim_idx_buf_idx, dispatch_meshlet_offset,\
  dispatch_meshlet_count, dispatch_instance_offset, dispatch_instance_count, base_vertex, idx32, payload,\
  out_vertices, placeholder2, out_indices)
#else
#define MESH_SHADER_CORE(\
  gid,\
  gtid,\
  meshlet_buf_idx,\
  vertex_idx_buf_idx,\
  prim_idx_buf_idx,\
  dispatch_meshlet_offset,\
  dispatch_meshlet_count,\
  dispatch_instance_offset,\
  dispatch_instance_count,\
  base_vertex,\
  idx32,\
  payload,\
  out_vertices,\
  out_primitives,\
  out_indices)\
MESH_SHADER_BODY(gid, gtid, meshlet_buf_idx, vertex_idx_buf_idx, prim_idx_buf_idx, dispatch_meshlet_offset,\
  dispatch_meshlet_count, dispatch_instance_offset, dispatch_instance_count, base_vertex, idx32, payload,\
  out_vertices, out_primitives, out_indices)
#endif
#endif


#define DECLARE_AMP_SHADER_MAIN(MainFuncName) [numthreads(AS_GROUP_SIZE, 1, 1)]\
  void MainFuncName(const uint dtid : SV_DispatchThreadID)


#ifdef MESH_SHADER_NO_PRIMITIVE_ATTRIBUTES
#define MESH_SHADER_PRIMITIVE_PARAM
#else
#define MESH_SHADER_PRIMITIVE_PARAM out primitives PrimitiveAttributes out_primitives[MESHLET_MAX_PRIMS],
#endif


#ifdef MESH_SHADER_NO_PAYLOAD
#define MESH_SHADER_PAYLOAD_PARAM
#else
#define MESH_SHADER_PAYLOAD_PARAM in payload CullingPayload payload,
#endif


#define DECLARE_MESH_SHADER_MAIN(MainFuncName)\
[outputtopology("triangle")]\
[numthreads(MS_THREAD_GROUP_SIZE, 1, 1)]\
void MainFuncName(\
  const uint gid : SV_GroupID,\
  const uint gtid : SV_GroupThreadID,\
  MESH_SHADER_PAYLOAD_PARAM\
  out vertices PsIn out_vertices[MESHLET_MAX_VERTS],\
  MESH_SHADER_PRIMITIVE_PARAM\
  out indices uint3 out_indices[MESHLET_MAX_PRIMS])


#endif
