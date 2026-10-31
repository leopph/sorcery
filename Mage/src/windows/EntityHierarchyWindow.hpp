#pragma once

#include "sorcery/observer_ptr.hpp"


namespace sorcery {
class World;


namespace mage {
class EditorApp;


class EntityHierarchyWindow {
public:
  explicit EntityHierarchyWindow(EditorApp& app);

  auto Draw() -> void;

private:
  ObserverPtr<EditorApp> app_;
};
}
}
