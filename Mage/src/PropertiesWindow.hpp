#pragma once

#include "EditorContext.hpp"


namespace sorcery::mage {
class PropertiesWindow {
  Context* mContext;
  bool mIsOpen{ true };

public:
  explicit PropertiesWindow(Context& context);
  auto Draw() -> void;
};
}
