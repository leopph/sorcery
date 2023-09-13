#include "Entity.hpp"

#include "../Resources/Scene.hpp"

#include <functional>
#include <iostream>
#include <cassert>
#include <format>

#include <imgui.h>
#include <imgui_stdlib.h>


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Entity>{"Entity"}
    .REFLECT_REGISTER_ENTITY_CTOR
    .property("components", &sorcery::Entity::mComponents);
}


namespace sorcery {
namespace {
std::vector<Entity*> gEntityCache;
}


auto Entity::FindEntityByName(std::string_view const name) -> Entity* {
  FindObjectsOfType(gEntityCache);
  for (auto* const entity : gEntityCache) {
    if (entity->GetName() == name) {
      return entity;
    }
  }
  return nullptr;
}


Entity::Entity() :
  mScene{Scene::GetActiveScene()} {}


auto Entity::GetScene() const -> Scene& {
  assert(mScene);
  return *mScene;
}


auto Entity::GetTransform() const -> TransformComponent& {
  if (!mTransform) {
    mTransform = GetComponent<TransformComponent>();
    assert(mTransform);
  }
  return *mTransform;
}


auto Entity::AddComponent(Component& component) -> void {
  component.SetEntity(*this);
  mComponents.emplace_back(&component);
}


auto Entity::RemoveComponent(Component& component) -> void {
  std::erase_if(mComponents, [&component](auto const storedComponent) {
    return storedComponent == &component;
  });
}


auto Entity::OnInit() -> void {
  SceneObject::OnInit();

  auto constexpr defaultEntityName{"New Entity"};
  SetName(defaultEntityName);
  FindObjectsOfType(gEntityCache);

  bool isNameUnique{false};
  std::size_t index{1};
  while (!isNameUnique) {
    isNameUnique = true;
    for (auto const entity : gEntityCache) {
      if (entity != this && entity->GetName() == GetName()) {
        SetName(std::format("{} ({})", defaultEntityName, index));
        ++index;
        isNameUnique = false;
        break;
      }
    }
  }

  mScene->AddEntity(*this);
}


auto Entity::OnDestroy() -> void {
  mScene->RemoveEntity(*this);

  // Entity destructor modifies this collection, hence the strange loop
  while (!mComponents.empty()) {
    Destroy(*mComponents.back());
  }

  SceneObject::OnDestroy();
}


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

  for (std::size_t i{0}; i < std::size(mComponents); i++) {
    auto const treeNodeId{std::format("{}##{}", rttr::type::get(*mComponents[i]).get_name().to_string(), std::to_string(i))};

    if (ImGui::TreeNodeEx(treeNodeId.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::Separator();
      if (ImGui::BeginTable("Component Property Table", 2, ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::PushItemWidth(FLT_MIN);
        ImGui::TableSetColumnIndex(1);
        ImGui::PushItemWidth(-FLT_MIN);
        ImGui::TableSetColumnIndex(0);

        mComponents[i]->OnDrawProperties(changed);
        ImGui::EndTable();
      }

      ImGui::TreePop();
    }

    if (ImGui::BeginPopupContextItem(treeNodeId.c_str())) {
      if (ImGui::MenuItem("Delete")) {
        auto const component{mComponents[i]};
        mComponents.erase(std::begin(mComponents) + i);
        Destroy(*component);
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
    for (auto const& componentClass : rttr::type::get<Component>().get_derived_classes()) {
      if (ImGui::MenuItem(componentClass.get_name().data())) {
        auto const component{componentClass.create().get_value<ObserverPtr<Component>>()};
        component->OnInit();
        AddComponent(*component);
        ImGui::CloseCurrentPopup();
      }
    }

    ImGui::EndPopup();
  }
}


auto Entity::OnDrawGizmosSelected() -> void {
  SceneObject::OnDrawGizmosSelected();

  for (auto const component : mComponents) {
    component->OnDrawGizmosSelected();
  }
}
}
