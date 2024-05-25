#include "Entity.hpp"

#include "../Resources/Scene.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <cassert>
#include <format>
#include <utility>

#include <imgui.h>
#include <imgui_stdlib.h>


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Entity>{"Entity"}
    .REFLECT_REGISTER_ENTITY_CTOR
    .property("components", &sorcery::Entity::GetComponentsForSerialization,
      &sorcery::Entity::SetComponentFromDeserialization);
}


namespace sorcery {
auto Entity::OnDrawProperties(bool& changed) -> void {
  SceneObject::OnDrawProperties(changed);

  static std::string entityName;
  entityName = GetName();

  if (ImGui::BeginTable("Property Widgets", 2)) {
    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::PushItemWidth(FLT_MIN);
    ImGui::Text("Name");

    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(-FLT_MIN);
    if (ImGui::InputText("##EntityName", &entityName, ImGuiInputTextFlags_EnterReturnsTrue)) {
      SetName(entityName);
    }

    ImGui::EndTable();
  }

  for (std::size_t i{0}; i < std::size(components_); i++) {
    auto const treeNodeId{
      std::format("{}##{}", rttr::type::get(*components_[i]).get_name().to_string(), std::to_string(i))
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

        components_[i]->OnDrawProperties(changed);
        ImGui::EndTable();
      }

      ImGui::TreePop();
    }

    if (ImGui::BeginPopupContextItem(treeNodeId.c_str())) {
      if (ImGui::MenuItem("Delete")) {
        RemoveComponent(*components_[i]);
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
      if (ImGui::MenuItem(component_class.get_name().data())) {
        auto component{Create(component_class)};
        AddComponent(std::unique_ptr<Component>{rttr::rttr_cast<Component*>(component.release())});
        ImGui::CloseCurrentPopup();
      }
    }

    ImGui::EndPopup();
  }
}


auto Entity::OnDrawGizmosSelected() -> void {
  SceneObject::OnDrawGizmosSelected();

  for (auto const& component : components_) {
    component->OnDrawGizmosSelected();
  }
}


auto Entity::Clone() -> std::unique_ptr<SceneObject> {
  return Create<Entity>(*this);
}


auto Entity::OnAfterEnteringScene(Scene const& scene) -> void {
  scene_.Reset(&scene);

  auto const entities{scene.GetEntities()};

  auto name{GetName()};

  for (std::size_t i{2}; true; i++) {
    auto name_is_unique{true};

    for (auto const& entity : entities) {
      if (entity.get() != this && entity->GetName() == name) {
        name_is_unique = false;
        break;
      }
    }

    if (name_is_unique) {
      break;
    }

    name = std::format("{} ({})", GetName(), i);
  }

  SetName(name);

  for (auto const& component : components_) {
    component->OnAfterEnteringScene(scene);
  }
}


auto Entity::OnBeforeExitingScene(Scene const& scene) -> void {
  for (auto const& component : components_) {
    component->OnBeforeExitingScene(scene);
  }

  scene_.Reset();
}


auto Entity::FindEntityByName(std::string_view const name) -> Entity* {
  static std::vector<Entity*> entities;
  FindObjectsOfType(entities);

  for (auto* const entity : entities) {
    if (entity->GetName() == name) {
      return entity;
    }
  }
  return nullptr;
}


auto Entity::GetComponentsForSerialization() const -> std::vector<Component*> {
  std::vector<Component*> ret;
  std::ranges::transform(components_, std::back_inserter(ret), [](auto const& component) {
    return component.get();
  });
  return ret;
}


auto Entity::SetComponentFromDeserialization(std::vector<Component*> components) -> void {
  while (!components_.empty()) {
    RemoveComponent(*components_.back());
  }

  for (auto* const component : components) {
    AddComponent(std::unique_ptr<Component>{component}); // Taking ownership
  }
}


Entity::Entity() {
  SetName("New Entity");
}


Entity::Entity(Entity const& other) :
  SceneObject{other} {
  SetName(other.GetName());
}


Entity::Entity(Entity&& other) noexcept :
  SceneObject{std::move(other)} {
  SetName(other.GetName());

  while (!other.components_.empty()) {
    AddComponent(other.RemoveComponent(*other.components_.back()));
  }
}


auto Entity::GetScene() const -> ObserverPtr<Scene const> {
  return scene_;
}


auto Entity::GetTransform() const -> TransformComponent& {
  if (!transform_) {
    transform_ = GetComponent<TransformComponent>();
  }
  return *transform_;
}


auto Entity::AddComponent(std::unique_ptr<Component> component) -> void {
  if (component) {
    components_.emplace_back(std::move(component));
    components_.back()->OnAfterAttachedToEntity(*this);

    if (scene_) {
      components_.back()->OnAfterEnteringScene(*scene_);
    }
  }
}


auto Entity::RemoveComponent(Component& component) -> std::unique_ptr<Component> {
  if (auto const it{
    std::ranges::find_if(components_, [&component](std::unique_ptr<Component> const& owned_component) {
      return owned_component.get() == std::addressof(component);
    })
  }; it != std::end(components_)) {
    if (scene_) {
      (*it)->OnBeforeExitingScene(*scene_);
    }

    (*it)->OnBeforeDetachedFromEntity(*this);

    auto ret{std::move(*it)};
    components_.erase(it);
    return ret;
  }

  return nullptr;
}
}
