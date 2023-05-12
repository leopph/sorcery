#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include "ObjectWrappers.hpp"


namespace leopph::editor {
auto ObjectWrapperFor<StaticMeshComponent>::OnDrawProperties([[maybe_unused]] Context& context, Object& object) -> void {
  auto& model{ dynamic_cast<StaticMeshComponent&>(object) };

  if (ImGui::BeginTable(std::format("{}", model.GetGuid().ToString()).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::PushItemWidth(FLT_MIN);
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(-FLT_MIN);

    ImGui::TableSetColumnIndex(0);
    ImGui::Text("%s", "Mesh");
    ImGui::TableNextColumn();

    static std::vector<Mesh*> meshes;
    static std::string meshFilter;

    if (ImGui::Button("Select##SelectMeshForStaticMeshComponent")) {
      Object::FindObjectsOfType(meshes);
      meshFilter.clear();
      ImGui::OpenPopup("ChooseMeshForStaticMeshComponent");
    }

    if (ImGui::BeginPopup("ChooseMeshForStaticMeshComponent")) {
      if (ImGui::InputText("###SearchMesh", &meshFilter)) {
        Object::FindObjectsOfType(meshes);
        std::erase_if(meshes, [](Mesh const* mesh) {
          return !Contains(mesh->GetName(), meshFilter);
        });
      }

      for (auto const mesh : meshes) {
        if (ImGui::Selectable(std::format("{}##meshoption{}", mesh->GetName(), mesh->GetGuid().ToString()).c_str())) {
          model.SetMesh(*mesh);
          break;
        }
      }

      ImGui::EndPopup();
    }

    ImGui::SameLine();
    ImGui::Text("%s", model.GetMesh().GetName().data());

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Materials");

    for (int i = 0; i < std::ssize(model.GetMesh().GetSubMeshes()); i++) {
      std::string const& mtlSlotName{ model.GetMesh().GetSubMeshes()[i].mtlSlotName };

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("%s", mtlSlotName.c_str());
      ImGui::TableNextColumn();

      static std::vector<Material*> allMaterials;
      static std::string matFilter;

      auto const popupId{ std::format("StaticMeshComponentMaterialSelectorPopup{}", i) };

      if (ImGui::Button(std::format("Select##Mtl{}", std::to_string(i)).c_str())) {
        Object::FindObjectsOfType(allMaterials);
        matFilter.clear();
        ImGui::OpenPopup(popupId.c_str());
      }

      if (ImGui::BeginPopup(popupId.c_str())) {
        if (ImGui::InputText("###SearchMat", &matFilter)) {
          Object::FindObjectsOfType(allMaterials);
          std::erase_if(allMaterials, [](Material const* mat) {
            return !Contains(mat->GetName(), matFilter);
          });
        }

        for (auto const mat : allMaterials) {
          if (ImGui::Selectable(std::format("{}##matoption{}", mat->GetName(), mat->GetGuid().ToString()).c_str())) {
            model.ReplaceMaterial(i, *mat);
            break;
          }
        }

        ImGui::EndPopup();
      }

      ImGui::SameLine();
      ImGui::Text("%s", model.GetMaterials()[i]->GetName().data());
    }

    ImGui::EndTable();
  }
}
}
