#pragma once

#include "Core.hpp"
#include "Serialization.hpp"
#include "Resources/Resource.hpp"

#include <concepts>
#include <filesystem>
#include <vector>


namespace sorcery {
class ResourceManager {
  struct ResourceGuidLess {
    using is_transparent = void;
    [[nodiscard]] LEOPPHAPI auto operator()(ObserverPtr<Resource> lhs, ObserverPtr<Resource> rhs) const noexcept -> bool;
    [[nodiscard]] LEOPPHAPI auto operator()(ObserverPtr<Resource> lhs, Guid const& rhs) const noexcept -> bool;
    [[nodiscard]] LEOPPHAPI auto operator()(Guid const& lhs, ObserverPtr<Resource> rhs) const noexcept -> bool;
  };


  std::set<ObserverPtr<Resource>, ResourceGuidLess> mResources;
  std::map<Guid, std::filesystem::path> mGuidPathMappings;

  [[nodiscard]] LEOPPHAPI auto InternalLoadResource(std::filesystem::path const& src) -> ObserverPtr<Resource>;

public:
  constexpr static std::string_view RESOURCE_META_FILE_EXT{".mojo"};

  template<std::derived_from<Resource> ResType = Resource>
  auto Load(Guid const& guid) -> ObserverPtr<ResType>;

  LEOPPHAPI auto Unload(Guid const& guid) -> void;

  [[nodiscard]] LEOPPHAPI auto IsLoaded(Guid const& guid) const -> bool;

  template<std::derived_from<Resource> ResType>
  auto Add(ObserverPtr<ResType> resource) -> void;

  LEOPPHAPI auto UpdateGuidPathMappings(std::map<Guid, std::filesystem::path> mappings) -> void;

  [[nodiscard]] LEOPPHAPI static auto GetMetaPath(std::filesystem::path const& path) -> std::filesystem::path;
  [[nodiscard]] LEOPPHAPI static auto IsMetaFile(std::filesystem::path const& path) -> bool;
};


LEOPPHAPI extern ResourceManager gResourceManager;


template<std::derived_from<Resource> ResType>
auto ResourceManager::Load(Guid const& guid) -> ObserverPtr<ResType> {
  if (auto const it{mResources.find(guid)}; it != std::end(mResources)) {
    if constexpr (!std::is_same_v<ResType, Resource>) {
      if (rttr::rttr_cast<ObserverPtr<ResType>>(*it)) {
        return static_cast<ObserverPtr<ResType>>(*it);
      }
    } else {
      return *it;
    }
  }

  if (auto const it{mGuidPathMappings.find(guid)}; it != std::end(mGuidPathMappings)) {
    auto const res{InternalLoadResource(it->second)};

    if constexpr (!std::is_same_v<ResType, Resource>) {
      if (rttr::rttr_cast<ObserverPtr<ResType>>(res)) {
        return static_cast<ObserverPtr<ResType>>(res);
      }
    } else {
      return res;
    }
  }

  return nullptr;
}


template<std::derived_from<Resource> ResType>
auto ResourceManager::Add(ObserverPtr<ResType> resource) -> void {
  if (resource && resource->GetGuid().IsValid()) {
    mResources.emplace(resource);
  }
}
}
