#pragma once

namespace sorcery::mage {
class EditorApp;


class PropertiesWindow {
public:
  explicit PropertiesWindow(EditorApp& app);
  auto Draw() const -> void;

private:
  EditorApp* app_;
};
}
