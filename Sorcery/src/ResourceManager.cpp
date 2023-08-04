#include "ResourceManager.hpp"
#include "Reflection.hpp"
#include "ResourceImporters/ResourceImporter.hpp"

#include <cassert>
#include <ranges>


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


auto ResourceManager::InternalLoadResource(std::filesystem::path const& resPathAbs) -> ObserverPtr<Resource> {
  Guid guid;
  std::unique_ptr<ResourceImporter> importer;

  if (!LoadMeta(resPathAbs, guid, importer)) {
    return nullptr;
  }

  if (auto const it{mResources.find(guid)}; it != std::end(mResources)) {
    return *it;
  }

  auto res{importer->Import(resPathAbs)};

  if (!res) {
    return nullptr;
  }

  res->SetName(resPathAbs.stem().string());
  res->SetGuid(guid);

  auto const [it, inserted]{mResources.emplace(res)};
  assert(inserted);

  return res;
}


auto ResourceManager::Unload(Guid const& guid) -> void {
  if (auto const it{mResources.find(guid)}; it != std::end(mResources)) {
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


auto ResourceManager::GetMetaPath(std::filesystem::path const& path) -> std::filesystem::path {
  return std::filesystem::path{path} += RESOURCE_META_FILE_EXT;
}


auto ResourceManager::IsMetaFile(std::filesystem::path const& path) -> bool {
  return path.extension() == RESOURCE_META_FILE_EXT;
}


auto ResourceManager::LoadMeta(std::filesystem::path const& resPathAbs, Guid& guid, std::unique_ptr<ResourceImporter>& importer) noexcept -> bool {
  auto const metaPathAbs{GetMetaPath(resPathAbs)};

  if (!exists(metaPathAbs)) {
    guid = Guid::Invalid();
    importer = nullptr;
    return false;
  }

  auto const metaNode{YAML::LoadFile(metaPathAbs.string())};
  guid = Guid::Parse(metaNode["guid"].as<std::string>());

  auto const importerType{rttr::type::get_by_name(metaNode["importer"]["type"].as<std::string>())};
  assert(importerType.is_valid());

  auto importerVariant{importerType.create()};
  assert(importerVariant.is_valid());

  importer.reset(importerVariant.get_value<ResourceImporter*>());
  ReflectionDeserializeFromYaml(metaNode["importer"]["properties"], *importer);
  return true;
}


auto ResourceManager::GetGuidsForResourcesOfType(rttr::type const& type, std::vector<Guid>& out) const noexcept -> void {
  for (auto const& resPathAbs : mGuidPathMappings | std::views::values) {
    Guid loadedGuid;
    std::unique_ptr<ResourceImporter> importer;

    if (gResourceManager.LoadMeta(resPathAbs, loadedGuid, importer) && importer->GetImportedType(resPathAbs) == type) {
      out.emplace_back(loadedGuid);
    }
  }
}
}
