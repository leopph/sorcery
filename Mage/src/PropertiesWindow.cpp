#include "PropertiesWindow.hpp"

#include "EditorApp.hpp"
#include "editor_gui.hpp"
#include "Reflection.hpp"

#include <limits>


namespace sorcery::mage {
PropertiesWindow::PropertiesWindow(EditorApp& app) :
  app_{&app} {}


auto PropertiesWindow::Draw() const -> void {
  ImGui::SetNextWindowSizeConstraints(ImVec2{300, 200}, ImVec2{
    std::numeric_limits<float>::max(), std::numeric_limits<float>::max()
  });

  if (ImGui::Begin("Object Properties")) {
    if (auto const selected_obj{app_->GetSelectedObject()}) {
      auto editable{true};

      if (auto const* const res{rttr::rttr_cast<Resource*>(selected_obj)};
        res && !app_->GetResourceDatabase().IsResourceEditable(res->GetId())) {
        editable = false;
      }

      auto changed{false};
      selected_obj->OnDrawProperties(editable, changed);

      if (changed) {
        if (auto const* const native_res{rttr::rttr_cast<NativeResource*>(selected_obj)};
          native_res && app_->GetResourceDatabase().IsSavedResource(*native_res)) {
          app_->GetResourceDatabase().SaveResourceToFile(*native_res);
        }
      }
    }
  }
  ImGui::End();
}
}
