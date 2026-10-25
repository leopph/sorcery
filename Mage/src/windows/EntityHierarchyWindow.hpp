#pragma once

#include "sorcery/observer_ptr.hpp"


namespace sorcery {
class World;


namespace mage {
class EditorApp;


class EntityHierarchyWindow {
public:
  EntityHierarchyWindow(EditorApp& app, World& edit_world);

  auto Draw() -> void;

private:
  ObserverPtr<EditorApp> app_;
  ObserverPtr<World> edit_world_;
};
}
}
