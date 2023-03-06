#pragma once

#include <Object.hpp>
#include "ObjectFactoryManager.hpp"

namespace leopph::editor {
struct AssetMetaInfo {
	Object::Type type;
	Guid guid;
	int importPrecedence;
};

[[nodiscard]] auto GenerateAssetMetaFileContents(Object const& asset, EditorObjectFactoryManager const& factoryManager) -> std::string;
[[nodiscard]] auto ReadAssetMetaFileContents(std::string const& contents) -> AssetMetaInfo;
}
