#include "EntityHierarchyWindow.hpp"

#include <functional>

#include "prefab.hpp"
#include "../editor_app.hpp"


namespace sorcery::mage {
EntityHierarchyWindow::EntityHierarchyWindow(EditorApp& app) :
  mApp{std::addressof(app)} {}


auto EntityHierarchyWindow::Draw() -> void {
  ImGui::SetNextWindowSizeConstraints(ImVec2{150, 150}, ImVec2{
    std::numeric_limits<float>::max(), std::numeric_limits<float>::max()
  });

  if (ImGui::Begin("Entities", nullptr, ImGuiWindowFlags_NoCollapse)) {
    if (ImGui::BeginChild("EntityHierarchyContent", ImVec2{0, 0}, ImGuiChildFlags_None, ImGuiWindowFlags_None)) {
      auto const contextId{"EntityHierarchyContextId"};

      if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow) && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup(contextId);
      }

      if (ImGui::BeginPopup(contextId)) {
        if (ImGui::MenuItem("Create New Entity")) {
          mApp->GetScene().AddEntity(std::make_unique<Entity>());
        }

        ImGui::EndPopup();
      }

      auto constexpr baseFlags{ImGuiTreeNodeFlags_OpenOnArrow};
      auto constexpr entityPayloadType{"ENTITY"};

      if (ImGui::BeginDragDropTarget()) {
        if (auto const payload{ImGui::AcceptDragDropPayload(entityPayloadType)}) {
          static_cast<Entity*>(payload->Data)->GetTransform().SetParent(nullptr);
          ImGui::EndDragDropTarget();
        }
      }

      std::span entities{mApp->GetScene().GetEntities()};

      std::function<void(Entity&)> displayEntityRecursive;
      displayEntityRecursive = [this, &entities, &displayEntityRecursive](Entity& entity) -> void {
        ImGuiTreeNodeFlags nodeFlags{baseFlags};

        if (entity.GetTransform().GetChildren().empty()) {
          nodeFlags |= ImGuiTreeNodeFlags_Leaf;
        } else {
          nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
        }

        if (mApp->GetSelectedObject() && mApp->GetSelectedObject() == &entity) {
          nodeFlags |= ImGuiTreeNodeFlags_Selected;
        }

        bool const nodeOpen{ImGui::TreeNodeEx(entity.GetName().data(), nodeFlags)};

        if (ImGui::BeginDragDropSource()) {
          ImGui::SetDragDropPayload(entityPayloadType, &entity, sizeof entity);
          ImGui::Text("%s", entity.GetName().data());
          ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget()) {
          if (auto const payload{ImGui::AcceptDragDropPayload(entityPayloadType)}) {
            static_cast<Entity*>(payload->Data)->GetTransform().SetParent(&entity.GetTransform());
          }
          ImGui::EndDragDropTarget();
        }

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
          mApp->SetSelectedObject(&entity);
        }

        auto deleted{false};

        if (ImGui::BeginPopupContextItem()) {
          mApp->SetSelectedObject(&entity);

          if (ImGui::MenuItem("Duplicate")) {
            auto clone{static_unique_ptr_cast<Entity>(entity.Clone())};
            mApp->SetSelectedObject(clone.get());
            mApp->GetScene().AddEntity(std::move(clone));
            entities = mApp->GetScene().GetEntities();
            ImGui::CloseCurrentPopup();
          }

          if (ImGui::MenuItem("Delete")) {
            mApp->SetSelectedObject(nullptr);
            mApp->GetScene().RemoveEntity(entity);
            entities = mApp->GetScene().GetEntities();
            deleted = true;
            ImGui::CloseCurrentPopup();
          }
          ImGui::EndPopup();
        }

        ImGui::OpenPopupOnItemClick(nullptr, ImGuiPopupFlags_MouseButtonRight);

        if (nodeOpen) {
          if (!deleted) {
            for (std::size_t childIndex{0}; childIndex < entity.GetTransform().GetChildren().size(); childIndex++) {
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
    ImGui::EndChild();

    if (ImGui::BeginDragDropTarget()) {
      if (auto const* const payload{ImGui::GetDragDropPayload()};
        payload && payload->IsDataType(ObjectDragDropPayload::kTypeStr.data())) {
        if (auto const* const data{static_cast<ObjectDragDropPayload const*>(payload->Data)}) {
          if (auto const* const prefab{rttr::rttr_cast<Prefab*>(data->ptr)}) {
            ImGui::AcceptDragDropPayload(ObjectDragDropPayload::kTypeStr.data());
            if (payload->IsDelivery()) {
              for (auto& entity : prefab->Instantiate()) {
                mApp->GetScene().AddEntity(std::move(entity));
              }
            }
          }
        }
      }
      ImGui::EndDragDropTarget();
    }
  }
  ImGui::End();
}
}
