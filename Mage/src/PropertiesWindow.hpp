#pragma once

#include "Application.hpp"


namespace sorcery::mage {
class PropertiesWindow {
  Application* mApp;
  bool mIsOpen{ true };

public:
  explicit PropertiesWindow(Application& app);
  auto Draw() -> void;
};
}
