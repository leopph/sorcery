#pragma once

#include "Core.hpp"
#include "Serialization.hpp"
#include "Resources/Resource.hpp"

#include <cstddef>
#include <concepts>
#include <filesystem>
#include <string_view>
#include <vector>


namespace sorcery {
class ResourceManager {
  struct ResourceGuidLess {
    using is_transparent = void;
    [[nodiscard]] LEOPPHAPI auto operator()(Resource* lhs, Resource* rhs) const noexcept -> bool;
    [[nodiscard]] LEOPPHAPI auto operator()(Resource* lhs, Guid const& rhs) const noexcept -> bool;
    [[nodiscard]] LEOPPHAPI auto operator()(Guid const& lhs, Resource* rhs) const noexcept -> bool;
  };

public:
  struct ResourceDescription {
    std::filesystem::path pathAbs;
    std::string name;
    rttr::type type;
  };

private:
  std::set<Resource*, ResourceGuidLess> mResources;
  std::map<Guid, ResourceDescription> mMappings;

  [[nodiscard]] LEOPPHAPI auto InternalLoadResource(Guid const& guid, ResourceDescription const& desc) -> Resource*;
  [[nodiscard]] static auto LoadTexture(std::span<std::byte const> bytes) noexcept -> MaybeNull<Resource*>;
  [[nodiscard]] static auto LoadMesh(std::span<std::byte const> bytes) -> MaybeNull<Resource*>;

public:
  constexpr static std::string_view EXTERNAL_RESOURCE_EXT{".bin"};
  constexpr static std::string_view SCENE_RESOURCE_EXT{".scene"};
  constexpr static std::string_view MATERIAL_RESOURCE_EXT{".mtl"};

  template<std::derived_from<Resource> ResType = Resource>
  auto GetOrLoad(Guid const& guid) -> ResType*;

  LEOPPHAPI auto Unload(Guid const& guid) -> void;

  [[nodiscard]] LEOPPHAPI auto IsLoaded(Guid const& guid) const -> bool;

  template<std::derived_from<Resource> ResType>
  auto Add(ResType* resource) -> void;

  LEOPPHAPI auto UpdateMappings(std::map<Guid, ResourceDescription> mappings) -> void;

  template<std::derived_from<Resource> T>
  auto GetGuidsForResourcesOfType(std::vector<Guid>& out) const noexcept -> void;
  auto LEOPPHAPI GetGuidsForResourcesOfType(rttr::type const& type, std::vector<Guid>& out) const noexcept -> void;
};


LEOPPHAPI extern ResourceManager gResourceManager;


template<std::derived_from<Resource> ResType>
auto ResourceManager::GetOrLoad(Guid const& guid) -> ResType* {
  if (auto const it{mResources.find(guid)}; it != std::end(mResources)) {
    if constexpr (!std::is_same_v<ResType, Resource>) {
      if (rttr::rttr_cast<ResType*>(*it)) {
        return static_cast<ResType*>(*it);
      }
    } else {
      return *it;
    }
  }

  if (auto const it{mMappings.find(guid)}; it != std::end(mMappings)) {
    if (auto const res{InternalLoadResource(guid, it->second)}) {
      if constexpr (!std::is_same_v<ResType, Resource>) {
        if (rttr::rttr_cast<ResType*>(res)) {
          return static_cast<ResType*>(res);
        }
      } else {
        return res;
      }
    }
  }

  return nullptr;
}


template<std::derived_from<Resource> ResType>
auto ResourceManager::Add(ResType* resource) -> void {
  if (resource && resource->GetGuid().IsValid()) {
    mResources.emplace(resource);
  }
}


template<std::derived_from<Resource> T>
auto ResourceManager::GetGuidsForResourcesOfType(std::vector<Guid>& out) const noexcept -> void {
  GetGuidsForResourcesOfType(rttr::type::get<T>(), out);
}
}
