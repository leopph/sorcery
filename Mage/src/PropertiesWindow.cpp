#include "PropertiesWindow.hpp"

#include "EditorApp.hpp"
#include "editor_gui.hpp"
#include "Reflection.hpp"

#include <limits>


namespace sorcery::mage {
PropertiesWindow::PropertiesWindow(EditorApp& app) :
  mApp{&app} {}


auto PropertiesWindow::Draw() -> void {
  ImGui::SetNextWindowSizeConstraints(ImVec2{300, 200}, ImVec2{
    std::numeric_limits<float>::max(), std::numeric_limits<float>::max()
  });

  if (ImGui::Begin("Object Properties")) {
    if (auto const selectedObj{mApp->GetSelectedObject()}) {
      bool changed{false};
      selectedObj->OnDrawProperties(changed);

      if (changed) {
        if (auto const nativeRes{rttr::rttr_cast<NativeResource*>(selectedObj)};
          nativeRes && mApp->GetResourceDatabase().IsSavedResource(*nativeRes)) {
          mApp->GetResourceDatabase().SaveResource(*nativeRes);
        }
      }
    }
  }
  ImGui::End();
}
}
