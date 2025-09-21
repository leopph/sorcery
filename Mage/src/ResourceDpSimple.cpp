#include "ResourceDbSimple.hpp"


namespace sorcery {
ResourceDbSimple::ResourceDbSimple(Object*& selected_obj_ptr) :
  selected_obj_ptr_{&selected_obj_ptr} {}


auto ResourceDbSimple::GetResourceDirectoryAbsolutePath() -> std::filesystem::path const& {
  return res_dir_path_abs_;
}
}
