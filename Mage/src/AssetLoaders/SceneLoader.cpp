#include "SceneLoader.hpp"

#include "Scene.hpp"

#include <fstream>
#include <vector>


namespace sorcery::mage {
auto SceneLoader::GetSupportedExtensions() const -> std::span<std::string const> {
  static std::array<std::string, 1> const extensions{
    "scene"
  };

  return extensions;
}


auto SceneLoader::Load(std::filesystem::path const& src, [[maybe_unused]] std::filesystem::path const& cache) -> std::unique_ptr<Object> {
  auto scene{ std::make_unique<Scene>() };

  if (std::ifstream in{ src, std::ios::in | std::ios::binary }; in.is_open()) {
    if (std::vector<std::uint8_t> fileData{ std::istreambuf_iterator{ in }, {} }; !fileData.empty()) {
      scene->Deserialize(fileData);
    }
  }

  return scene;
}


auto SceneLoader::GetPrecedence() const noexcept -> int {
  return 2;
}
}
