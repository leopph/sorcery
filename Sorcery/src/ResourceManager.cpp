#include "ResourceManager.hpp"
#include "Reflection.hpp"
#include "ResourceImporters/ResourceImporter.hpp"
#include "Serialization.hpp"
#undef FindResource

#include <cassert>
#include <fstream>


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


auto ResourceManager::LoadResource(std::filesystem::path const& src) -> std::weak_ptr<Resource> {
  if (!src.has_extension()) {
    return {};
  }

  if (auto const metaFilePath{ std::filesystem::path{ src } += RESOURCE_META_FILE_EXT }; exists(metaFilePath)) {
    // If there is a meta file we attempt to parse using it
    auto const metaNode{ YAML::LoadFile(metaFilePath.string()) };
    auto const guid{ Guid::Parse(metaNode["guid"].as<std::string>()) };
    auto const importerTypeStr{ metaNode["importer"]["type"] };
    auto const importerType{ rttr::type::get(importerTypeStr) };
    auto importerVariant{ importerType.create() };
    ReflectionDeserializeFromYAML(metaNode["importer"]["properties"], importerVariant);
    auto& importer{ importerVariant.get_value<ResourceImporter>() };
    auto res{ importer.Import(src) };
    res->SetGuid(guid);
    auto const [it, inserted]{ mResources.emplace(res) };
    assert(inserted);
    mPathToResource.emplace(absolute(src), it->get());
    mResourceToPath.emplace(it->get(), absolute(src));
    return *it;
  } else {
    // Otherwise we just find an importer for it
    for (auto const& derivedType : rttr::type::get<ResourceImporter>().get_derived_classes()) {
      static std::vector<std::string> supportedFileExtensions;
      supportedFileExtensions.clear();
      auto importerVariant{ derivedType.create() };
      auto& importer{ importerVariant.get_value<ResourceImporter>() };
      importer.GetSupportedFileExtensions(supportedFileExtensions);

      for (auto const& ext : supportedFileExtensions) {
        if (ext == src.extension()) {
          if (auto res{ importer.Import(src) }) {
            res->SetGuid(Guid::Generate());

            auto const [it, inserted]{ mResources.emplace(res) };
            assert(inserted);

            YAML::Node importerNode;
            importerNode["type"] = importerVariant.get_type().get_name().to_string();
            importerNode["properties"] = ReflectionSerializeToYAML(importerVariant);

            YAML::Node metaNode;
            metaNode["guid"] = static_cast<std::string>(res->GetGuid());
            metaNode["importer"] = importerNode;

            std::ofstream metaOutStream{ metaFilePath };
            YAML::Emitter emitter{ metaOutStream };
            emitter << metaNode;

            mPathToResource.emplace(absolute(src), it->get());
            mResourceToPath.emplace(it->get(), absolute(src));
            return *it;
          }
        }
      }
    }
    return {};
  }
}


auto ResourceManager::RemoveResource(Resource const& res) -> void {
  if (auto const it{ mResources.find(res.GetGuid()) }; it != std::end(mResources)) {
    mResources.erase(it);
  }
}


auto ResourceManager::FindResource(Guid const& guid) -> std::weak_ptr<Resource> {
  auto const it{ mResources.find(guid) };
  return it != std::end(mResources)
           ? *it
           : nullptr;
}
}
