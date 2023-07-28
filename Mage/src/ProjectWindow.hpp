#pragma once

#include "Application.hpp"

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


  Application* mContext;
  bool mIsOpen{ true };
  std::optional<std::filesystem::path> mSelectedNodePathProjDirRel; // nullopt is none is selected
  std::optional<RenameInfo> mRenameInfo; // nullopt if not renaming

  auto DrawFilesystemTree(std::filesystem::path const& rootDirAbs, std::filesystem::path const& thisNodePathRootRel, bool isThisNodeDirectory, Object* assetAtThisNode, std::optional<std::filesystem::path>& selectedNodePathRootRel) -> void;
  [[nodiscard]] static auto OpenFileDialog(std::string_view filters, std::string_view defaultPath, std::filesystem::path& out) -> bool;
  static auto ImportConcreteAsset(Application& context, AssetLoader& assetLoader, std::filesystem::path const& srcPathAbs, std::filesystem::path const& selectedDirAbs) -> void;
  static auto ImportAsset(Application& context, Object::Type targetAssetType, std::filesystem::path const& selectedDirAbs) -> void;
  static auto SaveNewNativeAsset(Application& context, std::unique_ptr<NativeResource> asset, std::string_view targetAssetFileName, std::filesystem::path const& selectedDirAbs) -> void;

public:
  explicit ProjectWindow(Application& context);

  auto Draw() -> void;
};
}
