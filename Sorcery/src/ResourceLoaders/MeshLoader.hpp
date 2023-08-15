#pragma once

#include "../Resources/Resource.hpp"

#include <filesystem>


namespace sorcery {
class MeshLoader {
  [[nodiscard]] LEOPPHAPI auto Load(std::filesystem::path const& pathAbs) -> MaybeNull<ObserverPtr<Resource>>;
};
}
