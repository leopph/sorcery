#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "Core.hpp"
#include "Math.hpp"


namespace sorcery {
// Skeleton nodes form a hierarchy in space and define transformations
struct SkeletonNode {
  std::string name;
  Matrix4 transform;
  std::optional<std::uint32_t> parent_idx;
};


// A bone is used for animations and references a skeleton node
struct Bone {
  Matrix4 offset_mtx;
  std::uint32_t skeleton_node_idx;
};


template<typename T>
struct AnimationKey {
  float timestamp;
  T value;
};


using AnimPositionKey = AnimationKey<Vector3>;
using AnimRotationKey = AnimationKey<Quaternion>;
using AnimScalingKey = AnimationKey<Vector3>;


// Node animations describe a transformation sequence for a specific skeleton node
struct NodeAnimation {
  std::vector<AnimPositionKey> position_keys;
  std::vector<AnimRotationKey> rotation_keys;
  std::vector<AnimScalingKey> scaling_keys;
  std::uint32_t node_idx;
};


// An animation is a list of node animations
struct Animation {
  std::string name;
  float duration;
  float ticks_per_second;
  std::vector<NodeAnimation> node_anims;
};


// A material slot is a named entry of a mesh
// where material instances can be applied
struct MaterialSlotInfo {
  std::string name;
};


// A meshlet is a small part of a submesh that can be rendered efficiently
struct MeshletData {
  std::uint32_t vert_count;
  std::uint32_t vert_offset;
  std::uint32_t prim_count;
  std::uint32_t prim_offset;
};


// Describes a triangle in a meshlet
struct MeshletTriangleData {
  std::uint32_t idx0 : 10;
  std::uint32_t idx1 : 10;
  std::uint32_t idx2 : 10;
};


// A submesh is a part of a mesh that can be rendered with a single material.
// It is composed of its own vertex data, a list of meshlets, and related indices.
struct SubmeshData {
  std::vector<Vector3> positions;
  std::vector<Vector3> normals;
  std::vector<Vector3> tangents;
  std::vector<Vector2> uvs;
  std::vector<Vector4> bone_weights;
  std::vector<Vector<std::uint32_t, 4>> bone_indices;
  std::vector<MeshletData> meshlets;
  std::vector<std::uint8_t> vertex_indices;
  std::vector<MeshletTriangleData> triangle_indices;
  std::uint32_t material_idx;
  bool idx32;
};


// A mesh is a list of submeshes, material slots, animations, and a skeleton
// Each submesh refers to a material slot and a set of bones.
// Bones refer to the skeleton hierarchy, and animations operate on the nodes of the hierarchy.
struct MeshData {
  std::vector<MaterialSlotInfo> material_slots;
  std::vector<SubmeshData> submeshes;
  std::vector<Animation> animations;
  std::vector<SkeletonNode> skeleton;
  std::vector<Bone> bones;
};


LEOPPHAPI extern std::uint16_t const kMeshletMaxVerts;
LEOPPHAPI extern std::uint16_t const kMeshletMaxPrims;
}
