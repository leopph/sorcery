#pragma once

#include "Util.hpp"

#include <utility>


namespace sorcery {
template<std::derived_from<Resource> ResType>
auto ResourceManager::GetOrLoad(Guid const& guid) -> ResType* {
  // Check default resources
  for (auto const& def_res : default_resources_) {
    if (def_res->GetGuid() == guid) {
      if constexpr (!std::is_same_v<ResType, Resource>) {
        return rttr::rttr_cast<ResType*>(def_res.Get());
      } else {
        return def_res.Get();
      }
    }
  }

  // Check loaded resources
  {
    auto const resources{loaded_resources_.LockShared()};

    if (auto const it{resources->find(guid)}; it != std::end(*resources)) {
      if constexpr (!std::is_same_v<ResType, Resource>) {
        return rttr::rttr_cast<ResType*>(it->get());
      } else {
        return it->get();
      }
    }
  }

  // Check in file mappings

  std::optional<ResourceDescription> desc;

  {
    auto const mappings{mappings_.LockShared()};
    if (auto const it{mappings->find(guid)}; it != std::end(*mappings)) {
      desc = it->second;
    }
  }

  // Load resource

  if (desc) {
    if (auto const res{InternalLoadResource(guid, *desc)}) {
      if constexpr (!std::is_same_v<ResType, Resource>) {
        return rttr::rttr_cast<ResType*>(res.Get());
      } else {
        return res.Get();
      }
    }
  }

  return nullptr;
}


template<std::derived_from<Resource> ResType>
auto ResourceManager::Add(std::unique_ptr<ResType> resource) -> ObserverPtr<ResType> {
  if (resource && resource->GetGuid().IsValid()) {
    return ObserverPtr{loaded_resources_.Lock()->emplace(std::move(resource)).first->get()};
  }

  return nullptr;
}


template<std::derived_from<Resource> ResType>
auto ResourceManager::Remove(Guid const& guid) -> std::unique_ptr<ResType> {
  auto resources{loaded_resources_.Lock()};

  if (auto const it{
    std::ranges::find_if(*resources, [&guid](std::unique_ptr<Resource> const& res) {
      if constexpr (std::is_same_v<ResType, Resource>) {
        return res->GetGuid() == guid;
      } else {
        return res->GetGuid() == guid && rttr::type::get(*res).get_raw_type().is_derived_from<ResType>();
      }
    })
  }; it != std::end(*resources)) {
    auto node{resources->extract(it)};
    return static_unique_ptr_cast<ResType>(std::move(node.value()));
  }

  return nullptr;
}


template<std::derived_from<Resource> T>
auto ResourceManager::GetGuidsForResourcesOfType(std::vector<Guid>& out) noexcept -> void {
  GetGuidsForResourcesOfType(rttr::type::get<T>(), out);
}


template<std::derived_from<Resource> T>
auto ResourceManager::GetInfoForResourcesOfType(std::vector<ResourceInfo>& out) -> void {
  GetInfoForResourcesOfType(rttr::type::get<T>(), out);
}
}
