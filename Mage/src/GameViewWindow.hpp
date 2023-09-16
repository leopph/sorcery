#pragma once

namespace sorcery::mage {
class GameViewWindow {
  int mResIdx{0};

public:
  auto Draw(bool gameRunning) -> void;
};
}
