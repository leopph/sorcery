#pragma once

#include "Application.hpp"


namespace sorcery::mage {
class EntityHierarchyWindow {
  Application* mApp;

public:
  explicit EntityHierarchyWindow(Application& app);

  auto Draw() -> void;
};


auto DrawEntityHierarchyWindow(Application& context) -> void;
}
