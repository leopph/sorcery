#include "StaticMeshComponent.hpp"

#include "Entity.hpp"
#include "Renderer.hpp"
#include "TransformComponent.hpp"

#include <format>
#include <stdexcept>

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::StaticMeshComponent>{ "Static Mesh Component" }
    .REFLECT_REGISTER_COMPONENT_CTOR;
}


namespace sorcery {
Object::Type const StaticMeshComponent::SerializationType{ Type::StaticMesh };


auto StaticMeshComponent::AdjustMaterialListForMesh() -> void {
  auto const mesh{ mMesh.Get() };

  if (!mesh) {
    return;
  }

  if (std::size_t const subMeshCount{ std::size(mesh->GetSubMeshes()) }, mtlCount{ std::size(mMaterials) }; subMeshCount != mtlCount) {
    mMaterials.resize(subMeshCount);

    for (std::size_t i{ mtlCount }; i < subMeshCount; i++) {
      mMaterials[i] = gRenderer.GetDefaultMaterial();
    }
  }
}


StaticMeshComponent::StaticMeshComponent() :
  mMesh{ gRenderer.GetCubeMesh() } {
  AdjustMaterialListForMesh();
  gRenderer.RegisterStaticMesh(this);
}


StaticMeshComponent::~StaticMeshComponent() {
  gRenderer.UnregisterStaticMesh(this);
}


auto StaticMeshComponent::GetMaterials() const noexcept -> std::span<ResourceHandle<Material> const> {
  return mMaterials;
}


auto StaticMeshComponent::SetMaterials(std::vector<ResourceHandle<Material>> materials) -> void {
  for (auto const& mtl : materials) {
    if (mtl == nullres) {
      throw std::runtime_error{ "Found nullptr while attempting to materials on StaticMeshComponent." };
    }
  }

  mMaterials = std::move(materials);
  AdjustMaterialListForMesh();
}


auto StaticMeshComponent::ReplaceMaterial(int const idx, ResourceHandle<Material> const& mtl) -> void {
  if (idx >= std::ssize(mMaterials)) {
    throw std::runtime_error{ std::format("Invalid index {} while attempting to replace material on StaticMeshComponent.", idx) };
  }

  mMaterials[idx] = mtl;
}


auto StaticMeshComponent::GetMesh() const noexcept -> ResourceHandle<Mesh> const& {
  return mMesh;
}


auto StaticMeshComponent::SetMesh(ResourceHandle<Mesh> const& mesh) noexcept -> void {
  if (mesh) {
    mMesh = mesh;
    AdjustMaterialListForMesh();
  }
}


auto StaticMeshComponent::GetSerializationType() const -> Type {
  return Type::StaticMesh;
}


auto StaticMeshComponent::CalculateBounds() const noexcept -> AABB {
  auto const mesh{ mMesh.Get() };

  if (!mesh) {
    return AABB{};
  }

  auto const& localBounds{ mesh->GetBounds() };
  auto const modelMtx{ GetEntity().GetTransform().GetModelMatrix() };
  auto boundsVertices{ localBounds.CalculateVertices() };

  for (auto& vertex : boundsVertices) {
    vertex = Vector3{ Vector4{ vertex, 1 } * modelMtx };
  }

  return AABB::FromVertices(boundsVertices);
}
}
