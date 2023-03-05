#pragma once

#include <Object.hpp>
#include "ObjectFactoryManager.hpp"

#include <memory>
#include <filesystem>
#include <span>

namespace leopph::editor {
struct AssetMetaInfo {
	Object::Type type;
	Guid guid;
	int importPrecedence;
};

[[nodiscard]] auto LoadAsset(std::filesystem::path const& filePath) -> std::shared_ptr<Object>;
[[nodiscard]] auto GenerateAssetMetaFileContents(Object const& asset, EditorObjectFactoryManager const& factoryManager) -> std::string;
[[nodiscard]] auto ReadAssetMetaFileContents(std::string const& contents) -> AssetMetaInfo;
auto AssignAssetMetaContents(Object& asset, std::span<u8 const> metaContentBytes) -> void;
}
