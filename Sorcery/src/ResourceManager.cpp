#include "ResourceManager.hpp"
#include "Reflection.hpp"
#include "ResourceImporters/ResourceImporter.hpp"

#include <cassert>


namespace sorcery {
ResourceManager gResourceManager;


auto ResourceManager::ResourceGuidLess::operator()(ObserverPtr<Resource> const lhs, ObserverPtr<Resource> const rhs) const noexcept -> bool {
  return lhs->GetGuid() < rhs->GetGuid();
}


auto ResourceManager::ResourceGuidLess::operator()(ObserverPtr<Resource> const lhs, Guid const& rhs) const noexcept -> bool {
  return lhs->GetGuid() < rhs;
}


auto ResourceManager::ResourceGuidLess::operator()(Guid const& lhs, ObserverPtr<Resource> const rhs) const noexcept -> bool {
  return lhs < rhs->GetGuid();
}


auto ResourceManager::InternalLoadResource(std::filesystem::path const& src) -> ObserverPtr<Resource> {
  auto const metaFilePath{ std::filesystem::path{ src } += RESOURCE_META_FILE_EXT };

  if (!exists(metaFilePath)) {
    return nullptr;
  }

  auto const metaNode{ YAML::LoadFile(metaFilePath.string()) };
  auto const guid{ Guid::Parse(metaNode["guid"].as<std::string>()) };

  if (auto const it{ mResources.find(guid) }; it != std::end(mResources)) {
    return *it;
  }

  auto const importerType{ rttr::type::get(metaNode["importer"]["type"]) };

  auto importerVariant{ importerType.create() };
  ReflectionDeserializeFromYaml(metaNode["importer"]["properties"], importerVariant);
  auto& importer{ importerVariant.get_value<ResourceImporter>() };

  auto res{ importer.Import(src) };

  if (!res) {
    return nullptr;
  }

  res->SetGuid(guid);

  auto const [it, inserted]{ mResources.emplace(res) };
  assert(inserted);

  return res;
}


auto ResourceManager::LoadResource(Guid const& guid) -> ObserverPtr<Resource> {
  return Load<Resource>(guid);
}


auto ResourceManager::Unload(Guid const& guid) -> void {
  if (auto const it{ mResources.find(guid) }; it != std::end(mResources)) {
    delete *it;
    mResources.erase(it);
  }
}


auto ResourceManager::IsLoaded(Guid const& guid) const -> bool {
  return mResources.contains(guid);
}


auto ResourceManager::UpdateGuidPathMappings(std::map<Guid, std::filesystem::path> mappings) -> void {
  mGuidPathMappings = std::move(mappings);
}
}
