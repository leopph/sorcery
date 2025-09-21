#include "ResourceDbSimple.hpp"

#include <ranges>

#include "EditorApp.hpp"


namespace sorcery {
ResourceDbSimple::ResourceDbSimple(Object*& selected_obj_ptr) :
  selected_obj_ptr_{&selected_obj_ptr} {}


auto ResourceDbSimple::Refresh() -> void {}


auto ResourceDbSimple::ChangeProjectDir(std::filesystem::path const& proj_dir_abs) -> void {
  if (!exists(proj_dir_abs)) {
    create_directory(proj_dir_abs);
  }

  res_dir_path_abs_ = proj_dir_abs / kResourceDirProjRel;

  if (!exists(res_dir_path_abs_)) {
    create_directory(res_dir_path_abs_);
  }

  for (auto const& guid : guid_to_path_abs_ | std::views::keys) {
    App::Instance().GetResourceManager().Unload(guid);
  }

  guid_to_path_abs_.clear();
  path_abs_to_guid_.clear();

  Refresh();
}


auto ResourceDbSimple::GetResourceDirectoryAbsolutePath() -> std::filesystem::path const& {
  return res_dir_path_abs_;
}


auto ResourceDbSimple::IsSavedResource(NativeResource const& res) const -> bool {
  return guid_to_path_abs_.contains(res.GetGuid());
}


auto ResourceDbSimple::PathToGuid(std::filesystem::path const& path_res_dir_rel) -> Guid {
  if (auto const it{path_abs_to_guid_.find(GetResourceDirectoryAbsolutePath() / path_res_dir_rel)};
    it != std::end(path_abs_to_guid_)) {
    return it->second;
  }

  return Guid::Invalid();
}


auto ResourceDbSimple::GuidToPath(Guid const& guid) -> std::filesystem::path {
  if (auto const it{guid_to_path_abs_.find(guid)}; it != std::end(guid_to_path_abs_)) {
    return relative(it->second, GetResourceDirectoryAbsolutePath());
  }

  return {};
}
}
