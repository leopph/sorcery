#pragma once

namespace sorcery::mage {
class EditorApp;


class PropertiesWindow {
  EditorApp* mApp;

public:
  explicit PropertiesWindow(EditorApp& app);
  auto Draw() -> void;
};
}
