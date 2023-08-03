#pragma once

#include "Application.hpp"

#include <filesystem>
#include <optional>
#include <string>


namespace sorcery::mage {
class ProjectWindow {
  struct RenameInfo {
    std::string newName;
    std::filesystem::path nodePathAbs;
  };


  struct ImportModalData {
    YAML::Node node;
    std::filesystem::path dstPathResDirRel;
  };


  Application* mApp;
  bool mIsOpen{true};
  std::filesystem::path mSelectedPathResDirRel; // empty if not selected
  std::optional<RenameInfo> mRenameInfo; // nullopt if not renaming
  ImportModalData mImportModal{};

  // Returns whether the drawn subtree was modified.
  [[nodiscard]] auto DrawFilesystemTree(std::filesystem::path const& resDirAbs, std::filesystem::path const& thisPathResDirRel) noexcept -> bool;
  [[nodiscard]] static auto OpenFileDialog(std::string_view filters, std::string_view defaultPath, std::filesystem::path& out) -> bool;

public:
  explicit ProjectWindow(Application& context);

  auto Draw() -> void;
};
}
