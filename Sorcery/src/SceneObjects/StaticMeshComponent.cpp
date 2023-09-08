#include "StaticMeshComponent.hpp"

#include "Entity.hpp"
#include "../Gui.hpp"
#include "../Rendering/Renderer.hpp"
#include "TransformComponent.hpp"

#include <imgui.h>

#include <format>
#include <stdexcept>

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::StaticMeshComponent>{"Static Mesh Component"}
    .REFLECT_REGISTER_COMPONENT_CTOR
    .property("mesh", &sorcery::StaticMeshComponent::mMesh)
    .property("materials", &sorcery::StaticMeshComponent::GetMaterials, &sorcery::StaticMeshComponent::SetMaterials);
}


namespace sorcery {
auto StaticMeshComponent::ResizeMaterialListToSubmeshCount() -> void {
  if (!mMesh) {
    mMaterials.clear();
    return;
  }

  if (auto const subMeshCount{std::size(mMesh->GetSubMeshes())}, mtlCount{std::size(mMaterials)}; subMeshCount != mtlCount) {
    mMaterials.resize(subMeshCount);

    for (std::size_t i{mtlCount}; i < subMeshCount; i++) {
      mMaterials[i] = gRenderer.GetDefaultMaterial();
    }
  }
}


StaticMeshComponent::StaticMeshComponent() :
  mMesh{gRenderer.GetCubeMesh()} {
  ResizeMaterialListToSubmeshCount();
}


auto StaticMeshComponent::GetMesh() const noexcept -> ObserverPtr<Mesh> {
  return mMesh;
}


auto StaticMeshComponent::SetMesh(ObserverPtr<Mesh> const mesh) noexcept -> void {
  mMesh = mesh;
  ResizeMaterialListToSubmeshCount();
}


auto StaticMeshComponent::GetMaterials() const noexcept -> std::vector<ObserverPtr<Material>> const& {
  return mMaterials;
}


auto StaticMeshComponent::SetMaterials(std::vector<ObserverPtr<Material>> const& materials) -> void {
  mMaterials = materials;
  ResizeMaterialListToSubmeshCount();
}


auto StaticMeshComponent::SetMaterial(int const idx, ObserverPtr<Material> const mtl) -> void {
  if (idx >= std::ssize(mMaterials)) {
    throw std::runtime_error{std::format("Invalid index {} while attempting to replace material on StaticMeshComponent.", idx)};
  }

  mMaterials[idx] = mtl;
}


auto StaticMeshComponent::CalculateBounds() const noexcept -> AABB {
  if (!mMesh) {
    return AABB{};
  }

  auto const& localBounds{mMesh->GetBounds()};
  auto const modelMtx{GetEntity().GetTransform().GetModelMatrix()};
  auto boundsVertices{localBounds.CalculateVertices()};

  for (auto& vertex : boundsVertices) {
    vertex = Vector3{Vector4{vertex, 1} * modelMtx};
  }

  return AABB::FromVertices(boundsVertices);
}


auto StaticMeshComponent::OnInit() -> void {
  Component::OnInit();
  gRenderer.Register(*this);
}


auto StaticMeshComponent::OnDestroy() -> void {
  gRenderer.Unregister(*this);
  Component::OnDestroy();
}


auto StaticMeshComponent::OnDrawProperties(bool& changed) -> void {
  Component::OnDrawProperties(changed);

  ImGui::Text("%s", "Mesh");
  ImGui::TableNextColumn();
  static ObjectPicker<Mesh> meshPicker;
  if (auto mesh{GetMesh()}; meshPicker.Draw(mesh, true)) {
    SetMesh(mesh);
  }

  if (auto const mesh{GetMesh()}) {
    ImGui::TableNextColumn();
    ImGui::Text("%s", "Materials");

    auto const mtlSlots{mesh->GetMaterialSlots()};
    auto const mtlCount{std::ssize(mtlSlots)};

    static std::vector<ObjectPicker<Material>> mtlPickers;
    if (std::ssize(mtlPickers) < mtlCount) {
      mtlPickers.resize(mtlCount);
    }

    auto const mtls{GetMaterials()};

    for (int i = 0; i < mtlCount; i++) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("%s", mtlSlots[i].name.c_str());
      ImGui::TableNextColumn();
      if (auto mtl{mtls[i]}; mtlPickers[i].Draw(mtl, true)) {
        SetMaterial(i, mtl);
      }
    }
  }
}
}
