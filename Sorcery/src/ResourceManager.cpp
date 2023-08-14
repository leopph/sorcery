#include "ResourceManager.hpp"

#include "MemoryAllocation.hpp"
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

  if (!LoadMeta(resPathAbs, std::addressof(guid), std::addressof(importer))) {
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


auto ResourceManager::GetGuidsForResourcesOfType(rttr::type const& type, std::vector<Guid>& out) const noexcept -> void {
  for (auto const& resPathAbs : mGuidPathMappings | std::views::values) {
    Guid loadedGuid;
    std::unique_ptr<ResourceImporter> importer;

    if (LoadMeta(resPathAbs, std::addressof(loadedGuid), std::addressof(importer)) && importer->GetImportedType(resPathAbs).is_derived_from(type)) {
      out.emplace_back(loadedGuid);
    }
  }

  // Resources that don't come from files
  for (auto const res : mResources) {
    auto contains{false};
    for (auto const& guid : out) {
      if (guid <=> res->GetGuid() == std::strong_ordering::equal) {
        contains = true;
        break;
      }
    }

    if (!contains && rttr::type::get(*res).is_derived_from(type)) {
      out.emplace_back(res->GetGuid());
    }
  }
}


auto ResourceManager::GetMetaPath(std::filesystem::path const& path) -> std::filesystem::path {
  return std::filesystem::path{path} += RESOURCE_META_FILE_EXT;
}


auto ResourceManager::IsMetaFile(std::filesystem::path const& path) -> bool {
  return path.extension() == RESOURCE_META_FILE_EXT;
}


auto ResourceManager::LoadMeta(std::filesystem::path const& resPathAbs, ObserverPtr<Guid> const guid, ObserverPtr<std::unique_ptr<ResourceImporter>> const importer) noexcept -> bool {
  auto const metaPathAbs{GetMetaPath(resPathAbs)};

  if (!exists(metaPathAbs)) {
    return false;
  }

  auto const metaNode{YAML::LoadFile(metaPathAbs.string())};

  if (guid) {
    *guid = Guid::Parse(metaNode["guid"].as<std::string>());
  }

  if (importer) {
    auto const importerType{rttr::type::get_by_name(metaNode["importer"]["type"].as<std::string>())};
    assert(importerType.is_valid());

    auto importerVariant{importerType.create()};
    assert(importerVariant.is_valid());

    importer->reset(importerVariant.get_value<ResourceImporter*>());
    ReflectionDeserializeFromYaml(metaNode["importer"]["properties"], **importer);
  }

  return true;
}


auto ResourceManager::GetNewImporterForResourceFile(std::filesystem::path const& path) -> std::unique_ptr<ResourceImporter> {
  for (auto const& importerType : rttr::type::get<ResourceImporter>().get_derived_classes()) {
    auto importerVariant{importerType.create()};
    std::unique_ptr<ResourceImporter> importer{importerVariant.get_value<ResourceImporter*>()};

    std::pmr::vector<std::string> supportedExtensions{&GetTmpMemRes()};
    importer->GetSupportedFileExtensions(supportedExtensions);

    for (auto const& ext : supportedExtensions) {
      if (ext == path.extension()) {
        return importer;
      }
    }
  }

  return {};
}
}
