#include "mesh_import.hpp"

#include "../Reflection.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::MeshImportSettings>("Mesh Import Settings")
    .constructor<>()(rttr::policy::ctor::as_object)
    .property("Fuse Submeshes", &sorcery::MeshImportSettings::fuse_submeshes)
    .property("Force 32-bit Indices", &sorcery::MeshImportSettings::force_idx32);
}
