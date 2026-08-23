#include "PropertiesWindow.hpp"

#include <limits>

#include "reflection.hpp"
#include "../editor_app.hpp"
#include "../gui_helpers.hpp"
#include "../drawers/editor_drawer_registry.hpp"


namespace sorcery::mage {
PropertiesWindow::PropertiesWindow(EditorApp& app, EditorDrawerRegistry& drawer_registry) :
  app_{&app},
  drawer_registry_{&drawer_registry} {}


auto PropertiesWindow::Draw() const -> void {
  ImGui::SetNextWindowSizeConstraints(ImVec2{300, 200}, ImVec2{
    std::numeric_limits<float>::max(), std::numeric_limits<float>::max()
  });

  if (ImGui::Begin("Object Properties")) {
    if (auto const selected_obj{app_->GetSelectedObject()}) {
      auto editable{true};

      if (auto const res{ReflCast<Resource>(selected_obj)};
        res && !app_->GetResourceDatabase().IsResourceEditable(res->GetResId())) {
        editable = false;
      }

      auto changed{false};
      drawer_registry_->Draw(*selected_obj, editable, changed);

      if (changed) {
        if (auto const native_res{ReflCast<NativeResource>(selected_obj)};
          native_res && app_->GetResourceDatabase().IsSavedResource(*native_res)) {
          app_->GetResourceDatabase().SaveResourceToFile(*native_res);
        }
      }
    }
  }
  ImGui::End();
}
}
