#pragma once

#include "Core.hpp"
#include "Resources/Resource.hpp"
#include "Resources/NativeResource.hpp"
#include "Serialization.hpp"

#include <concepts>
#include <filesystem>
#include <vector>


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

  LEOPPHAPI auto LoadResource(std::filesystem::path const& src, bool forceReload = false) -> std::weak_ptr<Resource>;
  LEOPPHAPI auto RemoveResource(Resource const& res) -> void;
#ifdef FindResource // <Windows.h> bullshit
#undef FindResource
#endif
  [[nodiscard]] LEOPPHAPI auto FindResource(Guid const& guid) -> std::weak_ptr<Resource>;

  template<std::derived_from<Resource> T>
  auto FindResourcesOfType(std::vector<std::weak_ptr<T>>& out) -> void;

  [[nodiscard]] LEOPPHAPI static auto GenerateMetaForNativeResource(NativeResource const& nativeRes) -> YAML::Node;
};


template<std::derived_from<Resource> T>
auto ResourceManager::FindResourcesOfType(std::vector<std::weak_ptr<T>>& out) -> void {
  for (auto const& res : mResources) {
    if (rttr::rttr_cast<T*>(res.get())) {
      out.emplace_back(std::static_pointer_cast<T>(res));
    }
  }
}


LEOPPHAPI extern ResourceManager gResourceManager;
}
