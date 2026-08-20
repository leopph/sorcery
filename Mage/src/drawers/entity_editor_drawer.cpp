#include "entity_editor_drawer.hpp"

#include <imgui.h>

#include "editor_drawer_registry.hpp"
#include "../gui_helpers.hpp"


namespace sorcery::mage {
auto EntityEditorDrawer::Draw(
  EditorDrawerContext const& ctx,
  Entity& obj,
  bool const allow_edit,
  bool& changed) -> void {
  ctx.registry->DrawAs<SceneObject>(obj, allow_edit, changed);

  static std::string entityName;
  entityName = obj.GetName();

  if (ImGui::BeginTable("Property Widgets", 2)) {
    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::PushItemWidth(FLT_MIN);
    ImGui::Text("Name");

    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(-FLT_MIN);
    if (ImGuiDisabled(!allow_edit, [&] {
      return ImGui::InputText("##EntityName", &entityName, ImGuiInputTextFlags_EnterReturnsTrue);
    })) {
      obj.SetName(entityName);
    }

    ImGui::EndTable();
  }

  auto const components{obj.GetComponents<Component>()};

  for (std::size_t i{0}; i < std::size(components); i++) {
    auto const treeNodeId{
      std::format("{}##{}", rttr::type::get(*components[i]).get_name().to_string(), std::to_string(i))
    };

    if (ImGui::TreeNodeEx(treeNodeId.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::Separator();
      if (ImGui::BeginTable("Component Property Table", 2, ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::PushItemWidth(FLT_MIN);
        ImGui::TableSetColumnIndex(1);
        ImGui::PushItemWidth(-FLT_MIN);
        ImGui::TableSetColumnIndex(0);

        ctx.registry->Draw(*components[i], allow_edit, changed);
        ImGui::EndTable();
      }

      ImGui::TreePop();
    }

    if (ImGui::BeginPopupContextItem(treeNodeId.c_str())) {
      if (ImGui::MenuItem("Delete", nullptr, false, allow_edit)) {
        obj.RemoveComponent(*components[i]);
        ImGui::EndPopup();
        break;
      }
      ImGui::EndPopup();
    }
    ImGui::OpenPopupOnItemClick(treeNodeId.c_str(), ImGuiPopupFlags_MouseButtonRight);
  }

  auto constexpr addNewComponentLabel = "Add New Component";
  ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(addNewComponentLabel).x) * 0.5f);
  ImGui::Button(addNewComponentLabel);

  if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
    for (auto const& component_class : rttr::type::get<Component>().get_derived_classes()) {
      if (component_class.get_constructors().empty()) {
        continue;
      }

      if (ImGui::MenuItem(component_class.get_name().data())) {
        obj.AddComponent(static_unique_ptr_cast<Component>(Create(component_class)));
        ImGui::CloseCurrentPopup();
      }
    }

    ImGui::EndPopup();
  }
}
}
