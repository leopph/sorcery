#include "PropertiesWindow.hpp"


namespace sorcery::mage {
PropertiesWindow::PropertiesWindow(Application& app) :
  mApp{ &app } { }


auto PropertiesWindow::Draw() -> void {
  if (ImGui::Begin("Object Properties", &mIsOpen)) {
    if (auto const selectedObj{ mApp->GetSelectedObject() }) {
      bool changed{ false };
      selectedObj->OnDrawProperties(changed);

      if (changed) {
        if (auto const nativeRes{ rttr::rttr_cast<ObserverPtr<NativeResource>>(selectedObj) }; nativeRes && mApp->GetResourceDatabase().IsSavedResource(*nativeRes)) {
          mApp->GetResourceDatabase().SaveResource(*nativeRes);
        }
      }
    }
  }
  ImGui::End();
}
}
