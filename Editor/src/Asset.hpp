#pragma once

#include <Object.hpp>

#include <memory>
#include <filesystem>
#include <span>

namespace leopph::editor {
std::shared_ptr<Object> LoadAsset(std::filesystem::path const& filePath);
std::string GenerateAssetMetaFileContents(Object const& asset);
void AssignAssetMetaContents(Object& asset, std::span<u8 const> metaContentBytes);
}
