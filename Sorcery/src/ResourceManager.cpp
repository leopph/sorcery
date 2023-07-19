#include "ResourceManager.hpp"

#include <cassert>


namespace sorcery {
ResourceManager gResourceManager;


auto ResourceManager::ResourceGuidLess::operator()(std::shared_ptr<Resource> const& lhs, std::shared_ptr<Resource> const& rhs) const noexcept -> bool {
  return lhs->GetGuid() < rhs->GetGuid();
}


auto ResourceManager::ResourceGuidLess::operator()(std::shared_ptr<Resource> const& lhs, Guid const& rhs) const noexcept -> bool {
  return lhs->GetGuid() < rhs;
}


auto ResourceManager::ResourceGuidLess::operator()(Guid const& lhs, std::shared_ptr<Resource> const& rhs) const noexcept -> bool {
  return lhs < rhs->GetGuid();
}


auto ResourceManager::AddResource(std::shared_ptr<Resource> res) -> std::weak_ptr<Resource> {
  auto const [it, insrted]{ mResources.emplace(std::move(res)) };
  assert(insrted);
  return *it;
}


auto ResourceManager::FindResource(Guid const& guid) -> std::weak_ptr<Resource> {
  auto const it{ mResources.find(guid) };
  return it != std::end(mResources)
           ? *it
           : nullptr;
}
}
