#pragma once

#include "EditorContext.hpp"

#include <filesystem>
#include <string_view>
#include <optional>
#include <string>


namespace sorcery::mage {
class ProjectWindow {
  constexpr static auto CONTEXT_MENU_ID{ "ProjectWindowContextMenu" };
  constexpr static auto CUBEMAP_IMPORT_MODAL_ID{ "ImportCubemap" };


  struct RenameInfo {
    std::string newName;
    std::filesystem::path nodePathAbs;
  };


  Context* mContext;
  bool mIsOpen{ true };
  std::optional<std::filesystem::path> mSelectedNodePathRootRel; // nullopt is none is selected
  std::optional<RenameInfo> mRenameInfo; // nullopt if not renaming

  auto DrawFilesystemTree(std::filesystem::path const& rootDirAbs, std::filesystem::path const& thisNodePathRootRel, bool isThisNodeDirectory, Object* assetAtThisNode, std::optional<std::filesystem::path>& selectedNodePathRootRel) -> void;
  [[nodiscard]] static auto OpenFileDialog(std::string_view filters, std::string_view defaultPath, std::filesystem::path& out) -> bool;
  static auto ImportConcreteAsset(Context& context, AssetLoader& assetLoader, std::filesystem::path const& srcPathAbs, std::filesystem::path const& selectedDirAbs) -> void;
  static auto ImportAsset(Context& context, Object::Type targetAssetType, std::filesystem::path const& selectedDirAbs) -> void;
  static auto SaveNewNativeAsset(Context& context, std::unique_ptr<NativeAsset> asset, std::string_view targetAssetFileName, std::filesystem::path const& selectedDirAbs) -> void;

public:
  explicit ProjectWindow(Context& context);

  auto Draw() -> void;
};
}
