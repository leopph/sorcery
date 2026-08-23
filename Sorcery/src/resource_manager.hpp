#pragma once

#include "Core.hpp"
#include "mutex.hpp"
#include "object_ptr.hpp"
#include "observer_ptr.hpp"
#include "resource_ref.hpp"
#include "resources/Material.hpp"
#include "resources/Mesh.hpp"
#include "Resources/Resource.hpp"

#include <concepts>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <string_view>
#include <vector>


namespace sorcery {
class JobSystem;
struct Job;


class ResourceManager {
public:
  struct ResourceDescription {
    std::string name;
    rttr::type type;
  };


  struct ResourceInfo {
    ResourceId id;
    std::string name;
    rttr::type type;
  };


  SORCERYAPI explicit ResourceManager(JobSystem& job_system);

  template<std::derived_from<Resource> ResType = Resource>
  auto Resolve(ResourceId const& res_id) -> ObjectPtr<ResType>;

  SORCERYAPI
  auto Unload(ResourceId const& res_id) -> void;

  SORCERYAPI
  auto UnloadAll() -> void;

  [[nodiscard]] SORCERYAPI
  auto IsLoaded(ResourceId const& res_id) -> bool;

  template<std::derived_from<Resource> ResType>
  auto Add(std::unique_ptr<ResType> resource) -> ResourceRef<ResType>;

  template<std::derived_from<Resource> ResType = Resource>
  [[nodiscard]]
  auto Remove(ResourceId const& res_id) -> std::unique_ptr<ResType>;

  SORCERYAPI
  auto UpdateMappings(std::map<ResourceId, ResourceDescription> res_mappings,
                      std::map<Guid, std::filesystem::path> file_mappings) -> void;

  template<std::derived_from<Resource> T>
  auto GetInfoForResourcesOfType(std::vector<ResourceInfo>& out) -> void;

  SORCERYAPI
  auto GetInfoForResourcesOfType(rttr::type const& type, std::vector<ResourceInfo>& out) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetDefaultMaterial() const noexcept -> ResourceRef<Material>;

  [[nodiscard]] SORCERYAPI
  auto GetCubeMesh() const noexcept -> ResourceRef<Mesh>;

  [[nodiscard]] SORCERYAPI
  auto GetPlaneMesh() const noexcept -> ResourceRef<Mesh>;

  [[nodiscard]] SORCERYAPI
  auto GetSphereMesh() const noexcept -> ResourceRef<Mesh>;

  // Called only internally!
  auto CreateDefaultResources() -> void;

  constexpr static std::string_view EXTERNAL_RESOURCE_EXT{".bin"};
  constexpr static std::string_view SCENE_RESOURCE_EXT{".scene"};
  constexpr static std::string_view MATERIAL_RESOURCE_EXT{".mtl"};

private:
  struct ResourceIdLess {
    using is_transparent = void;

    [[nodiscard]] SORCERYAPI auto operator()(std::unique_ptr<Resource> const& lhs,
                                             std::unique_ptr<Resource> const& rhs) const noexcept -> bool;

    [[nodiscard]] SORCERYAPI auto operator()(std::unique_ptr<Resource> const& lhs,
                                             ResourceId const& rhs) const noexcept -> bool;

    [[nodiscard]] SORCERYAPI auto operator()(ResourceId const& lhs,
                                             std::unique_ptr<Resource> const& rhs) const noexcept -> bool;
  };


  [[nodiscard]] SORCERYAPI auto InternalLoadResource(ResourceId const& res_id,
                                                     ResourceDescription const& desc) -> ObjectPtr<Resource>;
  [[nodiscard]] static
  auto LoadTexture(std::span<std::byte const> bytes) -> MaybeNull<std::unique_ptr<Resource>>;

  [[nodiscard]] static
  auto LoadMesh(std::span<std::byte const> bytes) -> MaybeNull<std::unique_ptr<Resource>>;

  [[nodiscard]] static
  auto LoadMaterial(std::span<std::byte const> bytes,
                    YamlDeserializeContext const& ctx) -> MaybeNull<std::unique_ptr<Resource>>;

  [[nodiscard]] static
  auto LoadPrefab(std::span<std::byte const> bytes,
                  YamlDeserializeContext const& ctx) -> MaybeNull<std::unique_ptr<Resource>>;

  ObserverPtr<JobSystem> job_system_;

  Mutex<std::set<std::unique_ptr<Resource>, ResourceIdLess>, true> loaded_resources_;

  std::unique_ptr<Material> default_mtl_;
  std::unique_ptr<Mesh> cube_mesh_;
  std::unique_ptr<Mesh> plane_mesh_;
  std::unique_ptr<Mesh> sphere_mesh_;

  // We can safely hold default resources with ObserverPtr
  // because we have owning pointers to them in other members
  std::vector<ObserverPtr<Resource>> default_resources_;

  Mutex<std::map<ResourceId, ResourceDescription>, true> res_mappings_;
  Mutex<std::map<Guid, std::filesystem::path>, true> file_mappings_;

  Mutex<std::map<ResourceId, ObserverPtr<Job>>, true> loader_jobs_;

  inline static Guid const default_res_guid_{1, 0};
  inline static ResourceId const default_mtl_res_id_{default_res_guid_, 0};
  inline static ResourceId const cube_mesh_res_id_{default_res_guid_, 1};
  inline static ResourceId const plane_mesh_res_id_{default_res_guid_, 2};
  inline static ResourceId const sphere_mesh_res_id_{default_res_guid_, 3};
};
}


#include "resource_manager.inl"
