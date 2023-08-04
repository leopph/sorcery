#include "StaticMeshComponent.hpp"

#include "Entity.hpp"
#include "Renderer.hpp"
#include "TransformComponent.hpp"
#include "Gui.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>

#include <format>
#include <cassert>
#include <stdexcept>

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::StaticMeshComponent>{"Static Mesh Component"}
    .REFLECT_REGISTER_COMPONENT_CTOR
    .property("mesh", &sorcery::StaticMeshComponent::mMesh)
    .property("materials", &sorcery::StaticMeshComponent::GetMaterials, &sorcery::StaticMeshComponent::SetMaterials);
}


namespace sorcery {
auto StaticMeshComponent::AdjustMaterialListForMesh() -> void {
  assert(mMesh);

  if (std::size_t const subMeshCount{std::size(mMesh->GetSubMeshes())}, mtlCount{std::size(mMaterials)}; subMeshCount != mtlCount) {
    mMaterials.resize(subMeshCount);

    for (std::size_t i{mtlCount}; i < subMeshCount; i++) {
      mMaterials[i] = gRenderer.GetDefaultMaterial();
    }
  }
}


StaticMeshComponent::StaticMeshComponent() :
  mMesh{gRenderer.GetCubeMesh()} {
  AdjustMaterialListForMesh();
  gRenderer.RegisterStaticMesh(this);
}


StaticMeshComponent::~StaticMeshComponent() {
  gRenderer.UnregisterStaticMesh(this);
}


auto StaticMeshComponent::GetMaterials() const noexcept -> std::vector<ObserverPtr<Material>> const& {
  return mMaterials;
}


auto StaticMeshComponent::SetMaterials(std::vector<ObserverPtr<Material>> const& materials) -> void {
  for (auto const mtl : materials) {
    if (!mtl) {
      throw std::runtime_error{"Found nullptr while attempting to materials on StaticMeshComponent."};
    }
  }

  mMaterials = materials;
  AdjustMaterialListForMesh();
}


auto StaticMeshComponent::ReplaceMaterial(int const idx, Material& mtl) -> void {
  if (idx >= std::ssize(mMaterials)) {
    throw std::runtime_error{std::format("Invalid index {} while attempting to replace material on StaticMeshComponent.", idx)};
  }

  mMaterials[idx] = std::addressof(mtl);
}


auto StaticMeshComponent::GetMesh() const noexcept -> Mesh& {
  assert(mMesh);
  return *mMesh;
}


auto StaticMeshComponent::SetMesh(Mesh& mesh) noexcept -> void {
  mMesh = std::addressof(mesh);
  AdjustMaterialListForMesh();
}


auto StaticMeshComponent::CalculateBounds() const noexcept -> AABB {
  assert(mMesh);

  auto const& localBounds{mMesh->GetBounds()};
  auto const modelMtx{GetEntity().GetTransform().GetModelMatrix()};
  auto boundsVertices{localBounds.CalculateVertices()};

  for (auto& vertex : boundsVertices) {
    vertex = Vector3{Vector4{vertex, 1} * modelMtx};
  }

  return AABB::FromVertices(boundsVertices);
}


auto StaticMeshComponent::OnDrawProperties(bool& changed) -> void {
  Component::OnDrawProperties(changed);

  ImGui::Text("%s", "Mesh");
  ImGui::TableNextColumn();
  static ObjectPicker<Mesh> meshPicker;
  if (auto mesh{std::addressof(GetMesh())}; meshPicker.Draw(mesh, false)) {
    assert(mesh);
    SetMesh(*mesh);
  }

  ImGui::TableNextColumn();
  ImGui::Text("%s", "Materials");

  auto const submeshCount{GetMesh().GetSubmeshCount()};

  static std::vector<ObjectPicker<Material>> mtlPickers;
  if (std::ssize(mtlPickers) < submeshCount) {
    mtlPickers.resize(submeshCount);
  }

  auto const mtls{GetMaterials()};
  for (int i = 0; i < submeshCount; i++) {
    std::string const& mtlSlotName{GetMesh().GetSubMeshes()[i].mtlSlotName};

    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text("%s", mtlSlotName.c_str());
    ImGui::TableNextColumn();
    if (auto mtl{mtls[i]}; mtlPickers[i].Draw(mtl, false)) {
      assert(mtl);
      ReplaceMaterial(i, *mtl);
    }
  }
}
}
