#pragma once

#include "Application.hpp"

#include <filesystem>
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
  std::filesystem::path mSelectedPathResDirRel; // empty if not selected
  std::optional<RenameInfo> mRenameInfo; // nullopt if not renaming

  auto DrawFilesystemTree(std::filesystem::path const& resDirAbs, std::filesystem::path const& thisPathResDirRel) -> void;
  [[nodiscard]] static auto OpenFileDialog(std::string_view filters, std::string_view defaultPath, std::filesystem::path& out) -> bool;

public:
  explicit ProjectWindow(Application& context);

  auto Draw() -> void;
};
}
