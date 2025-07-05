#ifndef MESH_SHADER_CORE_HLSLI
#define MESH_SHADER_CORE_HLSLI

#include "meshlet_culling.hlsli"
#include "shader_interop.h"


struct Meshlet {
  uint vertex_count;
  uint vertex_offset;
  uint primitive_count;
  uint primitive_offset;
};


uint3 UnpackTriangleIndices(uint const packed_indices) {
  return uint3(packed_indices & 0x3FF, (packed_indices >> 10) & 0x3FF, (packed_indices >> 20) & 0x3FF);
}


uint3 GetPrimitive(Meshlet const meshlet, uint const idx, StructuredBuffer<uint> const primitive_indices) {
  return UnpackTriangleIndices(primitive_indices[meshlet.primitive_offset + idx]);
}


uint GetVertexIndex(Meshlet const meshlet, uint const local_idx, uint const base_vertex, bool const idx32,
                    ByteAddressBuffer const vertex_indices) {
  uint const vertex_index_idx = meshlet.vertex_offset + local_idx;

  uint vertex_index;

  if (idx32) {
    vertex_index = vertex_indices.Load(vertex_index_idx * 4);
  } else {
    uint const word_offset = vertex_index_idx & 0x1;
    uint const byte_offset = (vertex_index_idx / 2) * 4;

    uint const index_pair = vertex_indices.Load(byte_offset);
    vertex_index = (index_pair >> (word_offset * 16)) & 0xFFFF;
  }

  return vertex_index + base_vertex;
}


void foo(uint vertex_idx_buf_idx) {
  Meshlet m;
  GetVertexIndex(m, 0, 0, true, ResourceDescriptorHeap[vertex_idx_buf_idx]);
}


#if !defined(MESH_SHADER_NO_PAYLOAD)
struct CullingPayload {
  uint meshlet_indices[AS_GROUP_SIZE];
};


groupshared CullingPayload g_payload;


void AmpShaderCore(
  uint const dtid,
  uint const dispatch_meshlet_offset,
  uint const dispatch_meshlet_count,
  uint const cull_data_buf_idx,
  uint const per_draw_cb_idx,
  uint const per_view_cb_idx) {
  bool visible = false;

  StructuredBuffer<MeshletCullData> const cull_data = ResourceDescriptorHeap[cull_data_buf_idx];
  ConstantBuffer<ShaderPerDrawConstants> const per_draw_cb = ResourceDescriptorHeap[per_draw_cb_idx];
  ConstantBuffer<ShaderPerViewConstants> const per_view_cb = ResourceDescriptorHeap[per_view_cb_idx];

  // Check bounds of meshlet cull data resource
  if (dtid < dispatch_meshlet_count) {
    // The actual index of the meshlet we are testing visibility for
    uint const meshlet_idx = dtid + dispatch_meshlet_offset;

    // Do visibility testing for this thread
    visible = IsMeshletVisible(cull_data[meshlet_idx], per_draw_cb.modelMtx, per_view_cb.frustum_planes_ws,
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
#define MESH_SHADER_BODY_GET_MESHLET_INDEX(gid, payload) gid
#else
#define MESH_SHADER_BODY_GET_MESHLET_INDEX(gid, payload) payload.meshlet_indices[gid]
#endif


#define MESH_SHADER_BODY(\
  gid,\
  gtid,\
  meshlet_buf_idx,\
  vertex_idx_buf_idx,\
  prim_idx_buf_idx,\
  dispatch_meshlet_offset,\
  dispatch_meshlet_count,\
  base_vertex,\
  idx32,\
  payload,\
  out_vertices,\
  out_primitives,\
  out_indices)\
uint const meshlet_idx = MESH_SHADER_BODY_GET_MESHLET_INDEX(gid, payload);\
StructuredBuffer<Meshlet> const meshlets = ResourceDescriptorHeap[meshlet_buf_idx];\
Meshlet const meshlet = meshlets[meshlet_idx + dispatch_meshlet_offset];\
\
SetMeshOutputCounts(meshlet.vertex_count, meshlet.primitive_count);\
\
for (uint i = 0; i < MS_VERTEX_LOOP_COUNT; i++) {\
  uint const vertex_id = gtid + i * MS_VERTEX_PRIMITIVE_LOOP_STRIDE;\
  \
  if (vertex_id < meshlet.vertex_count) {\
    uint const vertex_index = GetVertexIndex(meshlet, vertex_id, base_vertex, idx32, ResourceDescriptorHeap[vertex_idx_buf_idx]);\
    out_vertices[vertex_id] = VertexProcessor::CalculateVertex(vertex_index);\
  }\
}\
\
for (uint i = 0; i < MS_PRIMITIVE_LOOP_COUNT; i++) {\
  uint const primitive_id = gtid + i * MS_VERTEX_PRIMITIVE_LOOP_STRIDE;\
  \
  if (primitive_id < meshlet.primitive_count) {\
    out_indices[primitive_id] = GetPrimitive(meshlet, primitive_id, ResourceDescriptorHeap[prim_idx_buf_idx]);\
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
  base_vertex,\
  idx32,\
  out_vertices,\
  out_indices)\
MESH_SHADER_BODY(gid, gtid, meshlet_buf_idx, vertex_idx_buf_idx, prim_idx_buf_idx, dispatch_meshlet_offset,\
  dispatch_meshlet_count, base_vertex, idx32, placeholder1, out_vertices, placeholder2, out_indices)
#else
#define MESH_SHADER_CORE(\
  gid,\
  gtid,\
  meshlet_buf_idx,\
  vertex_idx_buf_idx,\
  prim_idx_buf_idx,\
  dispatch_meshlet_offset,\
  dispatch_meshlet_count,\
  base_vertex,\
  idx32,\
  out_vertices,\
  out_primitives,\
  out_indices)\
MESH_SHADER_BODY(gid, gtid, meshlet_buf_idx, vertex_idx_buf_idx, prim_idx_buf_idx, dispatch_meshlet_offset,\
  dispatch_meshlet_count, base_vertex, idx32, placeholder1, out_vertices, out_primitives, out_indices)
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
  base_vertex,\
  idx32,\
  payload,\
  out_vertices,\
  out_indices)\
MESH_SHADER_BODY(gid, gtid, meshlet_buf_idx, vertex_idx_buf_idx, prim_idx_buf_idx, dispatch_meshlet_offset,\
  dispatch_meshlet_count, base_vertex, idx32, payload, out_vertices, placeholder2, out_indices)
#else
#define MESH_SHADER_CORE(\
  gid,\
  gtid,\
  meshlet_buf_idx,\
  vertex_idx_buf_idx,\
  prim_idx_buf_idx,\
  dispatch_meshlet_offset,\
  dispatch_meshlet_count,\
  base_vertex,\
  idx32,\
  payload,\
  out_vertices,\
  out_primitives,\
  out_indices)\
MESH_SHADER_BODY(gid, gtid, meshlet_buf_idx, vertex_idx_buf_idx, prim_idx_buf_idx, dispatch_meshlet_offset,\
  dispatch_meshlet_count, base_vertex, idx32, payload, out_vertices, out_primitives, out_indices)
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
