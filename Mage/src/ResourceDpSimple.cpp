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


auto ResourceDbSimple::MoveResource(Guid const& guid, std::filesystem::path const& target_path_res_dir_rel) -> bool {
  auto const it{guid_to_path_abs_.find(guid)};

  if (it == std::end(guid_to_path_abs_)) {
    return false;
  }

  auto const src_path_abs{it->second};
  auto const dst_path_abs{res_dir_path_abs_ / target_path_res_dir_rel};

  if (!exists(src_path_abs) || exists(dst_path_abs)) {
    return false;
  }

  rename(src_path_abs, dst_path_abs);
  Refresh();

  return true;
}


auto ResourceDbSimple::MoveDirectory(std::filesystem::path const& src_path_res_dir_rel,
                                     std::filesystem::path const& dst_path_res_dir_rel) -> bool {
  auto const src_path_abs{weakly_canonical(GetResourceDirectoryAbsolutePath() / src_path_res_dir_rel)};
  auto const dst_path_abs{weakly_canonical(GetResourceDirectoryAbsolutePath() / dst_path_res_dir_rel)};

  if (!exists(src_path_abs) || exists(dst_path_abs) || !is_directory(src_path_abs) || equivalent(src_path_abs,
        GetResourceDirectoryAbsolutePath())) {
    return false;
  }

  rename(src_path_abs, dst_path_abs);
  Refresh();

  return true;
}


auto ResourceDbSimple::DeleteResource(Guid const& guid) -> void {
  App::Instance().GetResourceManager().Unload(guid);

  if (auto const it{guid_to_path_abs_.find(guid)}; it != std::end(guid_to_path_abs_)) {
    std::filesystem::remove(it->second);
    path_abs_to_guid_.erase(it->second);
    guid_to_path_abs_.erase(it);
  }

  guid_to_path_abs_.erase(guid);
  App::Instance().GetResourceManager().UpdateMappings(CreateMappings());
}


auto ResourceDbSimple::DeleteDirectory(std::filesystem::path const& path_res_dir_rel) -> bool {
  auto const pathAbs{weakly_canonical(GetResourceDirectoryAbsolutePath() / path_res_dir_rel)};

  if (!exists(pathAbs) || !is_directory(pathAbs)) {
    return false;
  }

  std::vector<Guid> resourcesToDelete;

  for (auto const& entry : std::filesystem::recursive_directory_iterator{pathAbs}) {
    if (auto const it{path_abs_to_guid_.find(entry.path())}; it != std::end(path_abs_to_guid_)) {
      resourcesToDelete.emplace_back(it->second);
    }
  }

  for (auto const& guid : resourcesToDelete) {
    DeleteResource(guid);
  }

  remove_all(pathAbs);
  return true;
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


auto ResourceDbSimple::CreateMappings() const noexcept -> std::map<Guid, ResourceManager::ResourceDescription> {
  return {};
}
}
