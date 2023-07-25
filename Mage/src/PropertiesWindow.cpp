#include "PropertiesWindow.hpp"


namespace sorcery::mage {
PropertiesWindow::PropertiesWindow(Context& context) :
  mContext{ &context } { }


auto PropertiesWindow::Draw() -> void {
  if (ImGui::Begin("Object Properties", &mIsOpen)) {
    if (auto const selectedObj{ mContext->GetSelectedObject() }) {
      mContext->GetFactoryManager().GetFor(selectedObj->GetSerializationType()).OnDrawProperties(*mContext, *selectedObj);
    }
  }
  ImGui::End();
}
}
