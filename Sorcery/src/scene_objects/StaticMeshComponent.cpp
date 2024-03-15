#include "StaticMeshComponent.hpp"

#include "Entity.hpp"
#include "../Gui.hpp"
#include "../engine_context.hpp"

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

  if (auto const subMeshCount{std::size(mMesh->GetSubMeshes())}, mtlCount{std::size(mMaterials)};
    subMeshCount != mtlCount) {
    mMaterials.resize(subMeshCount);

    for (std::size_t i{mtlCount}; i < subMeshCount; i++) {
      mMaterials[i] = g_engine_context.resource_manager->GetDefaultMaterial().Get();
    }
  }
}


StaticMeshComponent::StaticMeshComponent() :
  mMesh{g_engine_context.resource_manager->GetCubeMesh()} {
  ResizeMaterialListToSubmeshCount();
}


auto StaticMeshComponent::GetMesh() const noexcept -> Mesh* {
  return mMesh;
}


auto StaticMeshComponent::SetMesh(Mesh* const mesh) noexcept -> void {
  mMesh = mesh;
  ResizeMaterialListToSubmeshCount();
}


auto StaticMeshComponent::GetMaterials() const noexcept -> std::vector<Material*> const& {
  return mMaterials;
}


auto StaticMeshComponent::SetMaterials(std::vector<Material*> const& materials) -> void {
  mMaterials = materials;
  ResizeMaterialListToSubmeshCount();
}


auto StaticMeshComponent::SetMaterial(int const idx, Material* const mtl) -> void {
  if (idx >= std::ssize(mMaterials)) {
    throw std::runtime_error{
      std::format("Invalid index {} while attempting to replace material on StaticMeshComponent.", idx)
    };
  }

  mMaterials[idx] = mtl;
}


auto StaticMeshComponent::OnInit() -> void {
  Component::OnInit();
  g_engine_context.scene_renderer->Register(*this);
}


auto StaticMeshComponent::OnDestroy() -> void {
  g_engine_context.scene_renderer->Unregister(*this);
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
