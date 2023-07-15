#include <imgui.h>
#include <imgui_stdlib.h>

#include <rttr/registration>

#include "ObjectWrappers.hpp"
#include "Systems.hpp"
#include "../EditorContext.hpp"


namespace sorcery::mage {
auto ObjectWrapperFor<Entity>::OnDrawProperties(Context& context, Object& object) -> void {
  auto& entity{ dynamic_cast<Entity&>(object) };

  static std::string entityName;
  entityName = entity.GetName();

  if (ImGui::BeginTable("Property Widgets", 2)) {
    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::PushItemWidth(FLT_MIN);
    ImGui::Text("Name");

    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(-FLT_MIN);
    if (ImGui::InputText("##EntityName", &entityName)) {
      entity.SetName(entityName);
    }

    ImGui::EndTable();
  }

  static std::vector<Component*> components;
  entity.GetComponents(components);

  for (std::size_t i{ 0 }; i < std::size(components); i++) {
    auto const treeNodeId{ std::to_string(i) };

    if (ImGui::TreeNodeEx(treeNodeId.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::Separator();
      if (ImGui::BeginTable("Component Property Table", 2, ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::PushItemWidth(FLT_MIN);
        ImGui::TableSetColumnIndex(1);
        ImGui::PushItemWidth(-FLT_MIN);
        ImGui::TableSetColumnIndex(0);
        context.GetFactoryManager().GetFor(components[i]->GetSerializationType()).OnDrawProperties(context, *components[i]);
        ImGui::EndTable();
      }

      ImGui::TreePop();
    }

    if (ImGui::BeginPopupContextItem(treeNodeId.c_str())) {
      if (ImGui::MenuItem("Delete")) {
        entity.DestroyComponent(components[i]);
      }
      ImGui::EndPopup();
    }
    ImGui::OpenPopupOnItemClick(treeNodeId.c_str(), ImGuiPopupFlags_MouseButtonRight);
  }

  auto constexpr addNewComponentLabel = "Add New Component";
  ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(addNewComponentLabel).x) * 0.5f);
  ImGui::Button(addNewComponentLabel);

  if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
    for (auto const& componentClass : rttr::type::get_by_name("Component").get_derived_classes()) {
      if (ImGui::MenuItem(componentClass.get_name().data())) {
        auto component{ componentClass.create().get_value<std::shared_ptr<Component>>() };
        entity.AddComponent(std::move(component));
        ImGui::CloseCurrentPopup();
      }
    }

    ImGui::EndPopup();
  }
}


auto ObjectWrapperFor<Entity>::Instantiate() -> Object* {
  return &Scene::GetActiveScene()->CreateEntity();
}


auto ObjectWrapperFor<Entity>::OnDrawGizmosSelected(Context& context, Object& object) -> void {
  auto const& entity{ dynamic_cast<Entity&>(object) };
  std::vector<Component*> static components;
  components.clear();
  entity.GetComponents(components);

  for (auto const component : components) {
    context.GetFactoryManager().GetFor(component->GetSerializationType()).OnDrawGizmosSelected(context, *component);
  }
}
}
