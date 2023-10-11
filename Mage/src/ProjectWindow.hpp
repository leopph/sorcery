#pragma once

#include "Application.hpp"
#include "ResourceImporters/ResourceImporter.hpp"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>


namespace sorcery::mage {
class ProjectWindow {
  struct RenameInfo {
    std::string newName;
    std::filesystem::path nodePathAbs;
  };


  struct FileImportInfo {
    std::unique_ptr<ResourceImporter> importer;
    std::filesystem::path srcPathAbs;
    std::filesystem::path dstPathAbs;
  };


  Application* mApp;
  std::filesystem::path mSelectedPathResDirRel; // empty if not selected
  std::optional<RenameInfo> mRenameInfo; // nullopt if not renaming
  std::vector<FileImportInfo> mFilesToImport;
  bool mOpenImportModal{false};
  bool mOpenContextMenu{false};

  constexpr static std::string_view CONTEXT_MENU_ID{"ContextMenu"};
  constexpr static std::string_view DIR_NODE_DRAG_DROP_TYPE_STR{"NodeDragDropTypeStr"};

  // Returns whether the drawn subtree was modified.
  [[nodiscard]] auto DrawFilesystemTree(std::filesystem::path const& nodePathAbs, std::filesystem::path const& nodePathResDirRel, bool isDirectory) noexcept -> bool;
  auto DrawContextMenu() -> void;
  auto StartRenamingSelected() noexcept -> void;

public:
  explicit ProjectWindow(Application& context);

  auto Draw() -> void;
};
}
