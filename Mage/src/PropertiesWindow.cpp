#include "PropertiesWindow.hpp"


namespace sorcery::mage {
PropertiesWindow::PropertiesWindow(Application& app) :
  mApp{ &app } { }


auto PropertiesWindow::Draw() -> void {
  if (ImGui::Begin("Object Properties", &mIsOpen)) {
    if (auto const selectedObj{ mApp->GetSelectedObject() }) {
      selectedObj->OnDrawProperties();
    }
  }
  ImGui::End();
}
}
