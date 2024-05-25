#pragma once

#include <utility>


namespace sorcery {
template<std::derived_from<Resource> ResType>
auto ResourceManager::GetOrLoad(Guid const& guid) -> ResType* {
  {
    auto const resources{loaded_resources_.LockShared()};

    if (auto const it{resources->find(guid)}; it != std::end(*resources)) {
      if constexpr (!std::is_same_v<ResType, Resource>) {
        if (rttr::rttr_cast<ResType*>(it->get())) {
          return static_cast<ResType*>(it->get());
        }
      } else {
        return it->get();
      }
    }
  }

  std::optional<ResourceDescription> desc;

  {
    auto const mappings{mappings_.LockShared()};
    if (auto const it{mappings->find(guid)}; it != std::end(*mappings)) {
      desc = it->second;
    }
  }

  if (desc) {
    if (auto const res{InternalLoadResource(guid, *desc)}) {
      if constexpr (!std::is_same_v<ResType, Resource>) {
        if (rttr::rttr_cast<ResType*>(res.Get())) {
          return static_cast<ResType*>(res.Get());
        }
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


template<std::derived_from<Resource> T>
auto ResourceManager::GetGuidsForResourcesOfType(std::vector<Guid>& out) noexcept -> void {
  GetGuidsForResourcesOfType(rttr::type::get<T>(), out);
}


template<std::derived_from<Resource> T>
auto ResourceManager::GetInfoForResourcesOfType(std::vector<ResourceInfo>& out) -> void {
  GetInfoForResourcesOfType(rttr::type::get<T>(), out);
}
}