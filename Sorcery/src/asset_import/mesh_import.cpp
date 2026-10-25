#include "sorcery/host/assets/mesh_import.hpp"

#include "sorcery/reflection.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::MeshImportSettings>("Mesh Import Settings")
    .constructor<>()(rttr::policy::ctor::as_object)
    .property("Import As Single Mesh", &sorcery::MeshImportSettings::import_as_single_mesh)
    .property("Force 32-bit Indices", &sorcery::MeshImportSettings::force_idx32);
}
