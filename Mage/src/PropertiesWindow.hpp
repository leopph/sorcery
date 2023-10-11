#pragma once

#include "Application.hpp"


namespace sorcery::mage {
class PropertiesWindow {
  Application* mApp;

public:
  explicit PropertiesWindow(Application& app);
  auto Draw() -> void;
};
}
