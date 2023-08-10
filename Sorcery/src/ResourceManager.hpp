#pragma once

#include "Core.hpp"
#include "Serialization.hpp"
#include "ResourceImporters/ResourceImporter.hpp"
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

  [[nodiscard]] LEOPPHAPI auto InternalLoadResource(std::filesystem::path const& resPathAbs) -> ObserverPtr<Resource>;

public:
  constexpr static std::string_view RESOURCE_META_FILE_EXT{".mojo"};

  template<std::derived_from<Resource> ResType = Resource>
  auto GetOrLoad(Guid const& guid) -> ObserverPtr<ResType>;

  LEOPPHAPI auto Unload(Guid const& guid) -> void;

  [[nodiscard]] LEOPPHAPI auto IsLoaded(Guid const& guid) const -> bool;

  template<std::derived_from<Resource> ResType>
  auto Add(ObserverPtr<ResType> resource) -> void;

  LEOPPHAPI auto UpdateGuidPathMappings(std::map<Guid, std::filesystem::path> mappings) -> void;

  template<std::derived_from<Resource> T>
  auto GetGuidsForResourcesOfType(std::vector<Guid>& out) const noexcept -> void;
  auto LEOPPHAPI GetGuidsForResourcesOfType(rttr::type const& type, std::vector<Guid>& out) const noexcept -> void;

  [[nodiscard]] LEOPPHAPI static auto GetMetaPath(std::filesystem::path const& path) -> std::filesystem::path;
  [[nodiscard]] LEOPPHAPI static auto IsMetaFile(std::filesystem::path const& path) -> bool;
  // If the meta file successfully loads, guid and importer will be set to the read values.
  // Nullptrs can be passed to skip loading certain pieces of information.
  // The arguments won't be changed if the meta file failes to load.
  [[nodiscard]] LEOPPHAPI static auto LoadMeta(std::filesystem::path const& resPathAbs, ObserverPtr<Guid> guid, ObserverPtr<std::unique_ptr<ResourceImporter>> importer) noexcept -> bool;
  [[nodiscard]] LEOPPHAPI static auto GetNewImporterForResourceFile(std::filesystem::path const& path) -> std::unique_ptr<ResourceImporter>;
};


LEOPPHAPI extern ResourceManager gResourceManager;


template<std::derived_from<Resource> ResType>
auto ResourceManager::GetOrLoad(Guid const& guid) -> ObserverPtr<ResType> {
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
    if (auto const res{InternalLoadResource(it->second)}) {
      if constexpr (!std::is_same_v<ResType, Resource>) {
        if (rttr::rttr_cast<ObserverPtr<ResType>>(res)) {
          return static_cast<ObserverPtr<ResType>>(res);
        }
      } else {
        return res;
      }
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


template<std::derived_from<Resource> T>
auto ResourceManager::GetGuidsForResourcesOfType(std::vector<Guid>& out) const noexcept -> void {
  GetGuidsForResourcesOfType(rttr::type::get<T>(), out);
}
}
