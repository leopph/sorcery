#pragma once


namespace sorcery::mage {
class EditorApp;


class EntityHierarchyWindow {
  EditorApp* mApp;

public:
  explicit EntityHierarchyWindow(EditorApp& app);

  auto Draw() -> void;
};
}
