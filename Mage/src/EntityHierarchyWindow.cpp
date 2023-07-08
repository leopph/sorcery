#include "EntityHierarchyWindow.hpp"

#include <functional>


namespace sorcery::mage {
auto DrawEntityHierarchyWindow(Context& context) -> void {
  if (ImGui::Begin("Entities", nullptr, ImGuiWindowFlags_NoCollapse)) {
    auto const contextId{ "EntityHierarchyContextId" };

    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow) && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
      ImGui::OpenPopup(contextId);
    }

    if (ImGui::BeginPopup(contextId)) {
      if (ImGui::MenuItem("Create New Entity")) {
        auto& entity{ context.GetScene()->CreateEntity() };
        auto transform = std::make_unique<TransformComponent>();
        transform->SetGuid(Guid::Generate());
        entity.AddComponent(std::move(transform));
      }

      ImGui::EndPopup();
    }

    auto constexpr baseFlags{ ImGuiTreeNodeFlags_OpenOnArrow };
    auto constexpr entityPayloadType{ "ENTITY" };

    if (ImGui::BeginDragDropTarget()) {
      if (auto const payload{ ImGui::AcceptDragDropPayload(entityPayloadType) }) {
        static_cast<Entity*>(payload->Data)->GetTransform().SetParent(nullptr);
        ImGui::EndDragDropTarget();
      }
    }

    auto entities{ context.GetScene()->GetEntities() };

    std::function<void(Entity&)> displayEntityRecursive;
    displayEntityRecursive = [&context, &entities, &displayEntityRecursive](Entity& entity) -> void {
      ImGuiTreeNodeFlags nodeFlags{ baseFlags };

      if (entity.GetTransform().GetChildren().empty()) {
        nodeFlags |= ImGuiTreeNodeFlags_Leaf;
      } else {
        nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
      }

      if (context.GetSelectedObject() && context.GetSelectedObject()->GetGuid() == entity.GetGuid()) {
        nodeFlags |= ImGuiTreeNodeFlags_Selected;
      }

      bool const nodeOpen{ ImGui::TreeNodeEx(entity.GetName().data(), nodeFlags) };

      if (ImGui::BeginDragDropSource()) {
        ImGui::SetDragDropPayload(entityPayloadType, &entity, sizeof entity);
        ImGui::Text("%s", entity.GetName().data());
        ImGui::EndDragDropSource();
      }

      if (ImGui::BeginDragDropTarget()) {
        if (auto const payload{ ImGui::AcceptDragDropPayload(entityPayloadType) }) {
          static_cast<Entity*>(payload->Data)->GetTransform().SetParent(&entity.GetTransform());
        }
        ImGui::EndDragDropTarget();
      }

      if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        context.SetSelectedObject(&entity);
      }

      bool deleted{ false };

      if (ImGui::BeginPopupContextItem()) {
        context.SetSelectedObject(&entity);

        if (ImGui::MenuItem("Delete")) {
          entity.GetScene().DestroyEntity(entity);
          entities = context.GetScene()->GetEntities();
          context.SetSelectedObject(nullptr);
          deleted = true;
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }

      ImGui::OpenPopupOnItemClick(nullptr, ImGuiPopupFlags_MouseButtonRight);

      if (nodeOpen) {
        if (!deleted) {
          for (std::size_t childIndex{ 0 }; childIndex < entity.GetTransform().GetChildren().size(); childIndex++) {
            ImGui::PushID(static_cast<int>(childIndex));
            displayEntityRecursive(*entity.GetTransform().GetChildren()[childIndex]->GetEntity());
            ImGui::PopID();
          }
        }
        ImGui::TreePop();
      }
    };

    for (std::size_t i = 0; i < entities.size(); i++) {
      if (!entities[i]->GetTransform().GetParent()) {
        ImGui::PushID(static_cast<int>(i));
        displayEntityRecursive(*entities[i]);
        ImGui::PopID();
      }
    }
  }
  ImGui::End();
}
}
