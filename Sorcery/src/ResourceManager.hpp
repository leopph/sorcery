#pragma once

#include "Core.hpp"
#include "mutex.hpp"
#include "observer_ptr.hpp"
#include "Serialization.hpp"
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
    std::filesystem::path pathAbs;
    std::string name;
    rttr::type type;
  };


  struct ResourceInfo {
    Guid guid;
    std::string name;
    rttr::type type;
  };


  LEOPPHAPI explicit ResourceManager(JobSystem& job_system);

  template<std::derived_from<Resource> ResType = Resource>
  auto GetOrLoad(Guid const& guid) -> ResType*;

  LEOPPHAPI auto Unload(Guid const& guid) -> void;
  LEOPPHAPI auto UnloadAll() -> void;

  [[nodiscard]] LEOPPHAPI auto IsLoaded(Guid const& guid) -> bool;

  template<std::derived_from<Resource> ResType>
  auto Add(std::unique_ptr<ResType> resource) -> ObserverPtr<ResType>;

  template<std::derived_from<Resource> ResType = Resource>
  [[nodiscard]] auto Remove(Guid const& guid) -> std::unique_ptr<ResType>;

  LEOPPHAPI auto UpdateMappings(std::map<Guid, ResourceDescription> mappings) -> void;

  template<std::derived_from<Resource> T>
  auto GetGuidsForResourcesOfType(std::vector<Guid>& out) noexcept -> void;
  auto LEOPPHAPI GetGuidsForResourcesOfType(rttr::type const& type, std::vector<Guid>& out) noexcept -> void;

  template<std::derived_from<Resource> T>
  auto GetInfoForResourcesOfType(std::vector<ResourceInfo>& out) -> void;
  auto LEOPPHAPI GetInfoForResourcesOfType(rttr::type const& type, std::vector<ResourceInfo>& out) -> void;

  [[nodiscard]] LEOPPHAPI auto GetDefaultMaterial() const noexcept -> ObserverPtr<Material>;
  [[nodiscard]] LEOPPHAPI auto GetCubeMesh() const noexcept -> ObserverPtr<Mesh>;
  [[nodiscard]] LEOPPHAPI auto GetPlaneMesh() const noexcept -> ObserverPtr<Mesh>;
  [[nodiscard]] LEOPPHAPI auto GetSphereMesh() const noexcept -> ObserverPtr<Mesh>;

  // Called only internally!
  auto CreateDefaultResources() -> void;

  constexpr static std::string_view EXTERNAL_RESOURCE_EXT{".bin"};
  constexpr static std::string_view SCENE_RESOURCE_EXT{".scene"};
  constexpr static std::string_view MATERIAL_RESOURCE_EXT{".mtl"};

private:
  struct ResourceGuidLess {
    using is_transparent = void;

    [[nodiscard]] LEOPPHAPI auto operator()(std::unique_ptr<Resource> const& lhs,
                                            std::unique_ptr<Resource> const& rhs) const noexcept -> bool;

    [[nodiscard]] LEOPPHAPI auto operator()(std::unique_ptr<Resource> const& lhs,
                                            Guid const& rhs) const noexcept -> bool;

    [[nodiscard]] LEOPPHAPI auto operator()(Guid const& lhs,
                                            std::unique_ptr<Resource> const& rhs) const noexcept -> bool;
  };


  [[nodiscard]] LEOPPHAPI auto InternalLoadResource(Guid const& guid,
                                                    ResourceDescription const& desc) -> ObserverPtr<Resource>;
  [[nodiscard]] static auto LoadTexture(
    std::span<std::byte const> bytes) noexcept -> MaybeNull<std::unique_ptr<Resource>>;
  [[nodiscard]] static auto LoadMesh(std::span<std::byte const> bytes) -> MaybeNull<std::unique_ptr<Resource>>;

  inline static Guid const default_mtl_guid_{1, 0};
  inline static Guid const cube_mesh_guid_{2, 0};
  inline static Guid const plane_mesh_guid_{3, 0};
  inline static Guid const sphere_mesh_guid_{4, 0};

  Mutex<std::set<std::unique_ptr<Resource>, ResourceGuidLess>, true> loaded_resources_;
  std::vector<ObserverPtr<Resource>> default_resources_;
  Mutex<std::map<Guid, ResourceDescription>, true> mappings_;

  Mutex<std::map<Guid, ObserverPtr<Job>>, true> loader_jobs_;

  std::unique_ptr<Material> default_mtl_;
  std::unique_ptr<Mesh> cube_mesh_;
  std::unique_ptr<Mesh> plane_mesh_;
  std::unique_ptr<Mesh> sphere_mesh_;

  ObserverPtr<JobSystem> job_system_;
};
}


#include "resource_manager.inl"
