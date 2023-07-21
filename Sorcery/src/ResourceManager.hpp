#pragma once

#include "Core.hpp"
#include "Resources/Resource.hpp"

#include <filesystem>


namespace sorcery {
class ResourceManager {
  struct ResourceGuidLess {
    using is_transparent = void;
    [[nodiscard]] auto operator()(std::shared_ptr<Resource> const& lhs, std::shared_ptr<Resource> const& rhs) const noexcept -> bool;
    [[nodiscard]] auto operator()(std::shared_ptr<Resource> const& lhs, Guid const& rhs) const noexcept -> bool;
    [[nodiscard]] auto operator()(Guid const& lhs, std::shared_ptr<Resource> const& rhs) const noexcept -> bool;
  };


  std::set<std::shared_ptr<Resource>, ResourceGuidLess> mResources;
  std::map<std::filesystem::path, ObserverPtr<Resource>> mPathToResource;
  std::map<ObserverPtr<Resource>, std::filesystem::path> mResourceToPath;

public:
  constexpr static std::string_view RESOURCE_META_FILE_EXT{ ".mojo" };

  LEOPPHAPI auto LoadResource(std::filesystem::path const& src) -> std::weak_ptr<Resource>;
  LEOPPHAPI auto RemoveResource(Resource const& res) -> void;
  [[nodiscard]] LEOPPHAPI auto FindResource(Guid const& guid) -> std::weak_ptr<Resource>;
};


LEOPPHAPI extern ResourceManager gResourceManager;
}
