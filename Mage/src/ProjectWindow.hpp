#pragma once

#include "EditorContext.hpp"

#include <filesystem>
#include <string_view>


namespace sorcery::mage {
class ProjectWindow {
  Context* mContext;
  bool mIsOpen{ true };
  std::filesystem::path mSelectedDirRel{ "" };

  [[nodiscard]] static auto OpenFileDialog(std::string_view filters, std::string_view defaultPath, std::filesystem::path& out) -> bool;
  static auto ImportConcreteAsset(Context& context, AssetLoader& assetLoader, std::filesystem::path const& srcPathAbs, std::filesystem::path const& selectedDirAbs) -> void;
  static auto ImportAsset(Context& context, Object::Type targetAssetType, std::filesystem::path const& selectedDirAbs) -> void;
  static auto SaveNewNativeAsset(Context& context, std::unique_ptr<NativeAsset> asset, std::string_view targetAssetFileName, std::filesystem::path const& selectedDirAbs) -> void;

public:
  explicit ProjectWindow(Context& context);

  auto Draw() -> void;
};
}
