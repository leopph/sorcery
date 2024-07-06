#include "Mesh.hpp"

#include "../app.hpp"
#include "../Serialization.hpp"
#include "../rendering/render_manager.hpp"

#include <imgui.h>

#include <algorithm>
#include <cassert>
#include <utility>


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Mesh>{"Mesh"};
}


namespace sorcery {
auto Mesh::UploadToGpu() noexcept -> void {
  std::vector<Vector4> positions4{m_cpu_data_->positions.size()};
  std::ranges::transform(m_cpu_data_->positions, positions4.begin(), [](Vector3 const& p) {
    return Vector4{p, 1};
  });

  pos_buf_ = App::Instance().GetGraphicsDevice().CreateBuffer(graphics::BufferDesc{
    static_cast<UINT>(positions4.size() * sizeof(Vector4)), sizeof(Vector4), false, true, true
  }, D3D12_HEAP_TYPE_DEFAULT);
  assert(pos_buf_);

  App::Instance().GetRenderManager().UpdateBuffer(*pos_buf_, 0, as_bytes(std::span{positions4}));

  std::vector<Vector4> normals4{m_cpu_data_->normals.size()};
  std::ranges::transform(m_cpu_data_->normals, normals4.begin(), [](Vector3 const& n) {
    return Vector4{n, 0};
  });

  norm_buf_ = App::Instance().GetGraphicsDevice().CreateBuffer(graphics::BufferDesc{
    static_cast<UINT>(normals4.size() * sizeof(Vector4)), sizeof(Vector4), false, true, true
  }, D3D12_HEAP_TYPE_DEFAULT);
  assert(norm_buf_);

  App::Instance().GetRenderManager().UpdateBuffer(*norm_buf_, 0, as_bytes(std::span{normals4}));

  std::vector<Vector4> tangents4{m_cpu_data_->tangents.size()};
  std::ranges::transform(m_cpu_data_->tangents, tangents4.begin(), [](Vector3 const& t) {
    return Vector4{t, 0};
  });

  tan_buf_ = App::Instance().GetGraphicsDevice().CreateBuffer(graphics::BufferDesc{
    static_cast<UINT>(tangents4.size() * sizeof(Vector4)), sizeof(Vector4), false, true, false
  }, D3D12_HEAP_TYPE_DEFAULT);
  assert(norm_buf_);

  App::Instance().GetRenderManager().UpdateBuffer(*tan_buf_, 0, as_bytes(std::span{tangents4}));

  uv_buf_ = App::Instance().GetGraphicsDevice().CreateBuffer(graphics::BufferDesc{
    static_cast<UINT>(m_cpu_data_->uvs.size() * sizeof(Vector2)), sizeof(Vector2), false, true, false
  }, D3D12_HEAP_TYPE_DEFAULT);
  assert(uv_buf_);

  App::Instance().GetRenderManager().UpdateBuffer(*uv_buf_, 0, as_bytes(std::span{m_cpu_data_->uvs}));

  if (!m_cpu_data_->bone_weights.empty()) {
    bone_weight_buf_ = App::Instance().GetGraphicsDevice().CreateBuffer(graphics::BufferDesc{
      static_cast<UINT>(m_cpu_data_->bone_weights.size() * sizeof(Vector4)), sizeof(Vector4), false, true, true
    }, D3D12_HEAP_TYPE_DEFAULT);
    assert(bone_weight_buf_);

    App::Instance().GetRenderManager().UpdateBuffer(*bone_weight_buf_, 0,
      as_bytes(std::span{m_cpu_data_->bone_weights}));
  } else {
    bone_weight_buf_ = nullptr;
  }

  if (!m_cpu_data_->bone_indices.empty()) {
    bone_idx_buf_ = App::Instance().GetGraphicsDevice().CreateBuffer(graphics::BufferDesc{
      static_cast<UINT>(m_cpu_data_->bone_indices.size() * sizeof(Vector<std::uint32_t, 4>)),
      sizeof(Vector<std::uint32_t, 4>), false, true, true
    }, D3D12_HEAP_TYPE_DEFAULT);
    assert(bone_idx_buf_);

    App::Instance().GetRenderManager().UpdateBuffer(*bone_idx_buf_, 0, as_bytes(std::span{m_cpu_data_->bone_indices}));
  } else {
    bone_idx_buf_ = nullptr;
  }

  struct IdxBufInfo {
    UINT size;
    void* dataPtr;
  };

  auto const& [idxBufSize, idxBufDataPtr]{
    [this] {
      if (m_idx_format_ == DXGI_FORMAT_R16_UINT) {
        return IdxBufInfo{
          .size = static_cast<UINT>(std::size(m_cpu_data_->indices16) * sizeof(std::uint16_t)),
          .dataPtr = m_cpu_data_->indices16.data()
        };
      }
      if (m_idx_format_ == DXGI_FORMAT_R32_UINT) {
        return IdxBufInfo{
          .size = static_cast<UINT>(std::size(m_cpu_data_->indices32) * sizeof(std::uint32_t)),
          .dataPtr = m_cpu_data_->indices32.data()
        };
      }
      return IdxBufInfo{.size = 0, .dataPtr = nullptr};
    }()
  };

  idx_buf_ = App::Instance().GetGraphicsDevice().CreateBuffer(graphics::BufferDesc{idxBufSize, 0, false, false, false},
    D3D12_HEAP_TYPE_DEFAULT);
  assert(idx_buf_);

  App::Instance().GetRenderManager().UpdateBuffer(*idx_buf_, 0, std::span{
    std::bit_cast<std::byte const*>(idxBufDataPtr), idxBufSize
  });
}


auto Mesh::CalculateBounds() noexcept -> void {
  auto constexpr floatMin{std::numeric_limits<float>::lowest()};
  auto constexpr floatMax{std::numeric_limits<float>::max()};

  m_bounds_.min = Vector3{floatMax};
  m_bounds_.max = Vector3{floatMin};

  // TODO calculate bounds based on bones if they exist

  for (auto& submeshInfo : m_submeshes_) {
    submeshInfo.bounds.min = Vector3{floatMax};
    submeshInfo.bounds.max = Vector3{floatMin};

    for (auto i{0}; i < submeshInfo.index_count; i++) {
      std::uint32_t const idx{
        m_idx_format_ == DXGI_FORMAT_R16_UINT
          ? m_cpu_data_->indices16[i + submeshInfo.first_index]
          : m_cpu_data_->indices32[i + submeshInfo.first_index]
      };
      auto const& position{m_cpu_data_->positions[idx + submeshInfo.base_vertex]};

      m_bounds_.min = Min(m_bounds_.min, position);
      m_bounds_.max = Max(m_bounds_.max, position);

      submeshInfo.bounds.min = Min(submeshInfo.bounds.min, position);
      submeshInfo.bounds.max = Max(submeshInfo.bounds.max, position);
    }
  }
}


auto Mesh::EnsureCpuMemory() noexcept -> void {
  if (!m_cpu_data_) {
    m_cpu_data_ = std::make_unique<GeometryData>();
  }
}


auto Mesh::Set16BitIndicesFrom32BitBuffer(std::span<std::uint32_t const> const indices) noexcept -> void {
  assert(HasCpuMemory());
  m_cpu_data_->indices16.clear();
  std::ranges::transform(indices, std::back_inserter(m_cpu_data_->indices16), [](std::uint32_t const idx) {
    return static_cast<std::uint16_t>(idx);
  });
  m_cpu_data_->indices32.clear();
  m_idx_format_ = DXGI_FORMAT_R16_UINT;
}


Mesh::Mesh(Data data, bool const keep_data_in_cpu_memory) noexcept {
  SetData(std::move(data));
  [[maybe_unused]] auto const isValid{ValidateAndUpdate(keep_data_in_cpu_memory)};
  assert(isValid);
}


Mesh::~Mesh() {
  App::Instance().GetRenderManager().KeepAliveWhileInUse(pos_buf_);
  App::Instance().GetRenderManager().KeepAliveWhileInUse(norm_buf_);
  App::Instance().GetRenderManager().KeepAliveWhileInUse(tan_buf_);
  App::Instance().GetRenderManager().KeepAliveWhileInUse(uv_buf_);
  App::Instance().GetRenderManager().KeepAliveWhileInUse(bone_weight_buf_);
  App::Instance().GetRenderManager().KeepAliveWhileInUse(bone_idx_buf_);
  App::Instance().GetRenderManager().KeepAliveWhileInUse(idx_buf_);
}


auto Mesh::GetPositions() const noexcept -> std::span<Vector3 const> {
  return m_cpu_data_ ? m_cpu_data_->positions : std::span<Vector3 const>{};
}


auto Mesh::SetPositions(std::span<Vector3 const> positions) noexcept -> void {
  EnsureCpuMemory();
  m_cpu_data_->positions.assign(std::begin(positions), std::end(positions));
}


auto Mesh::SetPositions(std::vector<Vector3>&& positions) noexcept -> void {
  EnsureCpuMemory();
  m_cpu_data_->positions = std::move(positions);
}


auto Mesh::GetNormals() const noexcept -> std::span<Vector3 const> {
  return m_cpu_data_ ? m_cpu_data_->normals : std::span<Vector3 const>{};
}


auto Mesh::SetNormals(std::span<Vector3 const> normals) noexcept -> void {
  EnsureCpuMemory();
  m_cpu_data_->normals.assign(std::begin(normals), std::end(normals));
}


auto Mesh::SetNormals(std::vector<Vector3>&& normals) noexcept -> void {
  EnsureCpuMemory();
  m_cpu_data_->normals = std::move(normals);
}


auto Mesh::GetUVs() const noexcept -> std::span<Vector2 const> {
  return m_cpu_data_ ? m_cpu_data_->uvs : std::span<Vector2 const>{};
}


auto Mesh::SetUVs(std::span<Vector2 const> uvs) noexcept -> void {
  EnsureCpuMemory();
  m_cpu_data_->uvs.assign(std::begin(uvs), std::end(uvs));
}


auto Mesh::SetUVs(std::vector<Vector2>&& uvs) noexcept -> void {
  EnsureCpuMemory();
  m_cpu_data_->uvs = std::move(uvs);
}


auto Mesh::GetTangents() const noexcept -> std::span<Vector3 const> {
  return m_cpu_data_ ? m_cpu_data_->tangents : std::span<Vector3 const>{};
}


auto Mesh::SetTangents(std::span<Vector3 const> tangents) noexcept -> void {
  EnsureCpuMemory();
  m_cpu_data_->tangents.assign(std::begin(tangents), std::end(tangents));
}


auto Mesh::SetTangents(std::vector<Vector3>&& tangents) noexcept -> void {
  EnsureCpuMemory();
  m_cpu_data_->tangents = std::move(tangents);
}


auto Mesh::GetBoneWeights() const noexcept -> std::span<Vector4 const> {
  return m_cpu_data_ ? m_cpu_data_->bone_weights : std::span<Vector4 const>{};
}


auto Mesh::SetBoneWeights(std::span<Vector4 const> bone_weights) noexcept -> void {
  EnsureCpuMemory();
  m_cpu_data_->bone_weights.assign(std::begin(bone_weights), std::end(bone_weights));
}


auto Mesh::SetBoneWeights(std::vector<Vector4>&& bone_weights) noexcept -> void {
  EnsureCpuMemory();
  m_cpu_data_->bone_weights = std::move(bone_weights);
}


auto Mesh::GetBoneIndices() const noexcept -> std::span<Vector<std::uint32_t, 4> const> {
  return m_cpu_data_ ? m_cpu_data_->bone_indices : std::span<Vector<std::uint32_t, 4> const>{};
}


auto Mesh::SetBoneIndices(std::span<Vector<std::uint32_t, 4> const> bone_indices) noexcept -> void {
  EnsureCpuMemory();
  m_cpu_data_->bone_indices.assign(std::begin(bone_indices), std::end(bone_indices));
}


auto Mesh::SetBoneIndices(std::vector<Vector<std::uint32_t, 4>>&& bone_indices) noexcept -> void {
  EnsureCpuMemory();
  m_cpu_data_->bone_indices = std::move(bone_indices);
}


auto Mesh::GetIndices16() const noexcept -> std::span<std::uint16_t const> {
  return m_cpu_data_ ? m_cpu_data_->indices16 : std::span<std::uint16_t const>{};
}


auto Mesh::GetIndices32() const noexcept -> std::span<std::uint32_t const> {
  return m_cpu_data_ ? m_cpu_data_->indices32 : std::span<std::uint32_t const>{};
}


auto Mesh::SetIndices(std::span<std::uint16_t const> const indices) noexcept -> void {
  EnsureCpuMemory();
  m_cpu_data_->indices16.assign(std::begin(indices), std::end(indices));
  m_cpu_data_->indices32.clear();
  m_idx_format_ = DXGI_FORMAT_R16_UINT;
}


auto Mesh::SetIndices(std::span<std::uint32_t const> const indices) noexcept -> void {
  EnsureCpuMemory();

  for (auto const idx : indices) {
    if (idx > std::numeric_limits<std::uint16_t>::max()) {
      m_cpu_data_->indices16.clear();
      m_cpu_data_->indices32.assign(std::begin(indices), std::end(indices));
      m_idx_format_ = DXGI_FORMAT_R32_UINT;
      return;
    }
  }

  Set16BitIndicesFrom32BitBuffer(indices);
}


auto Mesh::SetIndices(std::vector<std::uint16_t>&& indices) noexcept -> void {
  EnsureCpuMemory();
  m_cpu_data_->indices16 = std::move(indices);
  m_cpu_data_->indices32.clear();
  m_idx_format_ = DXGI_FORMAT_R16_UINT;
}


auto Mesh::SetIndices(std::vector<std::uint32_t>&& indices) noexcept -> void {
  EnsureCpuMemory();

  for (auto const idx : indices) {
    if (idx > std::numeric_limits<std::uint16_t>::max()) {
      m_cpu_data_->indices16.clear();
      m_cpu_data_->indices32 = std::move(indices);
      m_idx_format_ = DXGI_FORMAT_R32_UINT;
      return;
    }
  }

  Set16BitIndicesFrom32BitBuffer(indices);
}


auto Mesh::GetMaterialSlots() const noexcept -> std::span<MaterialSlotInfo const> {
  return m_mtl_slots_;
}


auto Mesh::SetMaterialSlots(std::span<MaterialSlotInfo const> mtl_slots) noexcept -> void {
  m_mtl_slots_.assign(std::begin(mtl_slots), std::end(mtl_slots));
}


auto Mesh::GetSubMeshes() const noexcept -> std::span<SubMeshInfo const> {
  return m_submeshes_;
}


auto Mesh::SetSubMeshes(std::span<SubMeshInfo const> submeshes) noexcept -> void {
  m_submeshes_.assign(std::begin(submeshes), std::end(submeshes));
}


auto Mesh::SetSubmeshes(std::vector<SubMeshInfo>&& submeshes) noexcept -> void {
  m_submeshes_ = std::move(submeshes);
}


auto Mesh::GetAnimations() const noexcept -> std::span<Animation const> {
  return animations_;
}


auto Mesh::SetAnimations(std::span<Animation const> const animations) noexcept -> void {
  animations_.assign(std::begin(animations), std::end(animations));
}


auto Mesh::SetAnimations(std::vector<Animation>&& animations) noexcept -> void {
  animations_ = std::move(animations);
}


auto Mesh::GetSkeleton() const noexcept -> std::span<SkeletonNode const> {
  return skeleton_;
}


auto Mesh::SetSkeleton(std::span<SkeletonNode const> const skeleton) noexcept -> void {
  skeleton_.assign(std::begin(skeleton), std::end(skeleton));
}


auto Mesh::SetSkeleton(std::vector<SkeletonNode>&& skeleton) noexcept -> void {
  skeleton_ = std::move(skeleton);
}


auto Mesh::GetBones() const noexcept -> std::span<Bone const> {
  return bones_;
}


auto Mesh::SetBones(std::span<Bone const> bones) noexcept -> void {
  bones_.assign(std::begin(bones), std::end(bones));
}


auto Mesh::SetBones(std::vector<Bone>&& bones) noexcept -> void {
  bones_ = std::move(bones);
}


auto Mesh::GetBounds() const noexcept -> AABB const& {
  return m_bounds_;
}


auto Mesh::SetData(Data const& data) noexcept -> void {
  EnsureCpuMemory();
  SetPositions(data.positions);
  SetNormals(data.normals);
  SetUVs(data.uvs);
  SetTangents(data.tangents);
  SetBoneWeights(data.bone_weights);
  SetBoneIndices(data.bone_indices);
  std::visit([this]<typename T>(std::vector<T> const& indices) {
    SetIndices(indices);
  }, data.indices);
  SetMaterialSlots(data.material_slots);
  SetSubMeshes(data.sub_meshes);
  SetAnimations(data.animations);
  SetSkeleton(data.skeleton);
  SetBones(data.bones);
}


auto Mesh::SetData(Data&& data) noexcept -> void {
  EnsureCpuMemory();
  SetPositions(std::move(data.positions));
  SetNormals(std::move(data.normals));
  SetUVs(std::move(data.uvs));
  SetTangents(std::move(data.tangents));
  SetBoneWeights(std::move(data.bone_weights));
  SetBoneIndices(std::move(data.bone_indices));
  std::visit([this]<typename T>(std::vector<T>& indices) {
    SetIndices(std::move(indices));
  }, data.indices);
  SetMaterialSlots(std::move(data.material_slots));
  SetSubMeshes(std::move(data.sub_meshes));
  SetAnimations(std::move(data.animations));
  SetSkeleton(std::move(data.skeleton));
  SetBones(std::move(data.bones));
}


auto Mesh::ValidateAndUpdate(bool const keep_data_in_cpu_memory) noexcept -> bool {
  if (!m_cpu_data_) {
    return true;
  }

  // TODO validate bone weights and indices

  if (m_cpu_data_->positions.size() != m_cpu_data_->normals.size() || m_cpu_data_->normals.size() != m_cpu_data_->uvs.
      size() || m_cpu_data_->uvs.size() != m_cpu_data_->tangents.size() || m_cpu_data_->positions.empty() || (
        m_cpu_data_->indices16.empty() && m_cpu_data_->indices32.empty())) {
    return false;
  }

  for (auto const& [baseVertex, firstIdx, idxCount, mtlSlotIdx, bounds] : m_submeshes_) {
    if (baseVertex >= std::ssize(m_cpu_data_->positions)) {
      return false;
    }

    if (firstIdx >= std::ssize(m_cpu_data_->indices16) && firstIdx >= std::ssize(m_cpu_data_->indices32)) {
      return false;
    }

    auto const lastIdx{firstIdx + idxCount - 1};

    if (lastIdx >= std::ssize(m_cpu_data_->indices16) && lastIdx >= std::ssize(m_cpu_data_->indices32)) {
      return false;
    }

    for (auto i{firstIdx}; i <= lastIdx; i++) {
      if (auto const idx{std::ssize(m_cpu_data_->indices16) > i ? m_cpu_data_->indices16[i] : m_cpu_data_->indices32[i]}
        ; idx + baseVertex > std::size(m_cpu_data_->positions)) {
        return false;
      }
    }

    if (mtlSlotIdx < 0 || mtlSlotIdx >= std::ssize(m_mtl_slots_)) {
      return false;
    }
  }

  m_vertex_count_ = static_cast<int>(std::ssize(m_cpu_data_->positions));
  m_index_count_ = static_cast<int>(m_cpu_data_->indices16.empty()
                                      ? std::ssize(m_cpu_data_->indices32)
                                      : std::ssize(m_cpu_data_->indices16));
  m_submesh_count_ = static_cast<int>(std::ssize(m_submeshes_));

  CalculateBounds();
  UploadToGpu();

  if (!keep_data_in_cpu_memory) {
    ReleaseCpuMemory();
  }

  return true;
}


auto Mesh::HasCpuMemory() const noexcept -> bool {
  return static_cast<bool>(m_cpu_data_);
}


auto Mesh::ReleaseCpuMemory() noexcept -> void {
  m_cpu_data_.reset();
}


auto Mesh::GetPositionBuffer() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return pos_buf_;
}


auto Mesh::GetNormalBuffer() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return norm_buf_;
}


auto Mesh::GetUvBuffer() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return uv_buf_;
}


auto Mesh::GetTangentBuffer() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return tan_buf_;
}


auto Mesh::GetBoneWeightBuffer() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return bone_weight_buf_;
}


auto Mesh::GetBoneIndexBuffer() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return bone_idx_buf_;
}


auto Mesh::GetIndexBuffer() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return idx_buf_;
}


auto Mesh::GetVertexCount() const noexcept -> int {
  return m_vertex_count_;
}


auto Mesh::GetIndexCount() const noexcept -> int {
  return m_index_count_;
}


auto Mesh::GetSubmeshCount() const noexcept -> int {
  return m_submesh_count_;
}


auto Mesh::GetIndexFormat() const noexcept -> DXGI_FORMAT {
  return m_idx_format_;
}


auto Mesh::OnDrawProperties(bool& changed) -> void {
  Resource::OnDrawProperties(changed);

  ImGui::Text("%s: %d", "Vertex Count", GetVertexCount());
  ImGui::Text("%s: %d", "Index Count", GetIndexCount());
  ImGui::Text("%s: %d", "Submesh Count", GetSubmeshCount());
}
}
