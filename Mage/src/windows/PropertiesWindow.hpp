#pragma once

#include "observer_ptr.hpp"


namespace sorcery::mage {
class EditorApp;
class EditorDrawerRegistry;


class PropertiesWindow {
public:
  explicit PropertiesWindow(EditorApp& app, EditorDrawerRegistry& drawer_registry);
  auto Draw() const -> void;

private:
  ObserverPtr<EditorApp> app_;
  ObserverPtr<EditorDrawerRegistry> drawer_registry_;
};
}
