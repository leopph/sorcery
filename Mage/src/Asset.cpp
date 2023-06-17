#include "Asset.hpp"

#include <YamlInclude.hpp>


namespace sorcery::mage {
auto GenerateAssetMetaFileContents(Object const& asset, ObjectWrapperManager const& factoryManager) -> std::string {
  YAML::Node node;
  node["guid"] = asset.GetGuid().ToString();
  node["type"] = static_cast<int>(asset.GetSerializationType());
  node["importPrecedence"] = factoryManager.GetFor(asset.GetSerializationType()).GetLoader().GetPrecedence();
  return Dump(node);
}


auto ReadAssetMetaFileContents(std::string const& contents) -> AssetMetaInfo {
  auto const node{ YAML::Load(contents) };
  if (!node["guid"] || !node["type"] || !node["importPrecedence"]) {
    throw std::runtime_error{ "Corrupt asset meta info." };
  }
  return AssetMetaInfo{ static_cast<Object::Type>(node["type"].as<int>()), Guid::Parse(node["guid"].as<std::string>()), node["importPrecedence"].as<int>() };
}
}
