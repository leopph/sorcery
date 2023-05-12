#pragma once

#include <Object.hpp>
#include "ObjectWrappers/ObjectWrapperManager.hpp"


namespace leopph::editor {
struct AssetMetaInfo {
  Object::Type type;
  Guid guid;
  int importPrecedence;
};


[[nodiscard]] auto GenerateAssetMetaFileContents(Object const& asset, ObjectWrapperManager const& factoryManager) -> std::string;
[[nodiscard]] auto ReadAssetMetaFileContents(std::string const& contents) -> AssetMetaInfo;
}
