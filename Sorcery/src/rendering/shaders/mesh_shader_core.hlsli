#ifndef MESH_SHADER_CORE_HLSLI
#define MESH_SHADER_CORE_HLSLI

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


#define THREAD_GROUP_SIZE MESHLET_MAX_VERTS
#define VERTEX_LOOP_COUNT (MESHLET_MAX_VERTS + THREAD_GROUP_SIZE - 1) / THREAD_GROUP_SIZE
#define PRIMITIVE_LOOP_COUNT (MESHLET_MAX_PRIMS + THREAD_GROUP_SIZE - 1) / THREAD_GROUP_SIZE
#define VERTEX_PRIMITIVE_LOOP_STRIDE THREAD_GROUP_SIZE


template<typename VertexProcessor,
#if !defined(MESH_SHADER_NO_PRIMITIVE_ATTRIBUTES)
         typename PrimitiveProcessor,
         typename PerTriData,
#endif
         typename PsIn>
void MeshShaderCore(
  uint const gid,
  uint const gtid,
  uint const meshlet_buf_idx,
  uint const vertex_idx_buf_idx,
  uint const prim_idx_buf_idx,
  uint const draw_meshlet_offset,
  uint const draw_meshlet_count,
  uint const draw_instance_offset,
  uint const draw_instance_count,
  uint const base_vertex,
  bool const idx32,
  out PsIn out_vertices[MESHLET_MAX_VERTS],
#if !defined(MESH_SHADER_NO_PRIMITIVE_ATTRIBUTES)
  out PerTriData out_primitives[MESHLET_MAX_PRIMS],
#endif
  out uint3 out_indices[MESHLET_MAX_PRIMS]) {
  uint const meshlet_idx = gid / draw_instance_count;
  StructuredBuffer<Meshlet> const meshlets = ResourceDescriptorHeap[meshlet_buf_idx];
  Meshlet const meshlet = meshlets[meshlet_idx + draw_meshlet_offset];

  uint start_instance = gid % draw_instance_count;
  uint instance_count = 1;

  if (meshlet_idx == draw_meshlet_count - 1) {
    uint const instances_per_group = min(MESHLET_MAX_VERTS / meshlet.vertex_count,
      MESHLET_MAX_PRIMS / meshlet.primitive_count);

    uint const unpacked_group_count = (draw_meshlet_count - 1) * draw_instance_count;
    uint const packed_index = gid - unpacked_group_count;

    start_instance = packed_index * instances_per_group;
    instance_count = min(draw_instance_count - start_instance, instances_per_group);
  }

  uint const vert_count = meshlet.vertex_count * instance_count;
  uint const prim_count = meshlet.primitive_count * instance_count;

  SetMeshOutputCounts(vert_count, prim_count);

  for (uint i = 0; i < VERTEX_LOOP_COUNT; i++) {
    uint const vertex_id = gtid + i * THREAD_GROUP_SIZE;

    if (vertex_id < vert_count) {
      uint const read_index = vertex_id % meshlet.vertex_count;
      uint const instance_id = vertex_id / meshlet.vertex_count;

      uint vertex_index;

      if (idx32) {
        StructuredBuffer<uint> const vertex_indices = ResourceDescriptorHeap[vertex_idx_buf_idx];
        vertex_index = vertex_indices[meshlet.vertex_offset + read_index] + base_vertex;
      } else {
        StructuredBuffer<uint16_t> const vertex_indices = ResourceDescriptorHeap[vertex_idx_buf_idx];
        vertex_index = vertex_indices[meshlet.vertex_offset + read_index] + base_vertex;
      }

      uint const instance_index = start_instance + instance_id;

      out_vertices[vertex_id] = VertexProcessor::CalculateVertex(vertex_index, instance_index);
    }
  }

  for (uint i = 0; i < PRIMITIVE_LOOP_COUNT; i++) {
    uint const primitive_id = gtid + i * VERTEX_PRIMITIVE_LOOP_STRIDE;

    if (primitive_id < prim_count) {
      uint const read_index = primitive_id % meshlet.primitive_count;
      uint const instance_id = primitive_id / meshlet.primitive_count;

      StructuredBuffer<uint> const primitive_indices = ResourceDescriptorHeap[prim_idx_buf_idx];

      out_indices[primitive_id] = UnpackTriangleIndices(primitive_indices[meshlet.primitive_offset + read_index])
                                  + (meshlet.vertex_count * instance_id);

#if !defined(MESH_SHADER_NO_PRIMITIVE_ATTRIBUTES)
      out_primitives[primitive_id] = PrimitiveProcessor::CalculatePrimitive(primitive_id);
#endif
    }
  }
}


#ifdef MESH_SHADER_NO_PRIMITIVE_ATTRIBUTES
#define DECLARE_MESH_SHADER_MAIN(MainFuncName) [outputtopology("triangle")]\
[numthreads(THREAD_GROUP_SIZE, 1, 1)]\
void MainFuncName(\
  const uint gid : SV_GroupID, \
  const uint gtid : SV_GroupThreadID, \
  out vertices PsIn out_vertices[MESHLET_MAX_VERTS], \
  out indices uint3 out_indices[MESHLET_MAX_PRIMS])
#else
#define DECLARE_MESH_SHADER_MAIN(MainFuncName, PrimitiveDataType) [outputtopology("triangle")]\
[numthreads(THREAD_GROUP_SIZE, 1, 1)]\
void MainFuncName(\
  const uint gid : SV_GroupID,\
  const uint gtid : SV_GroupThreadID,\
  out vertices PsIn out_vertices[MESHLET_MAX_VERTS],\
  out primitives PrimitiveDataType out_primitives[MESHLET_MAX_PRIMS],\
  out indices uint3 out_indices[MESHLET_MAX_PRIMS])
#endif

#endif
