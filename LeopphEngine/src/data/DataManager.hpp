#pragma once

#include "../components/Behavior.hpp"
#include "../components/Component.hpp"
#include "../components/lighting/DirLight.hpp"
#include "../components/lighting/PointLight.hpp"
#include "../components/lighting/SpotLight.hpp"
#include "../components/rendering/RenderComponent.hpp"
#include "../entity/Entity.hpp"
#include "../rendering/Skybox.hpp"
#include "../rendering/SkyboxImpl.hpp"
#include "../rendering/Texture.hpp"
#include "../rendering/geometry/GlMeshGroup.hpp"
#include "../rendering/geometry/MeshDataGroup.hpp"
#include "../util/equal/PathedEqual.hpp"
#include "../util/hash/PathedHash.hpp"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


namespace leopph::internal
{
	// A DataManager object holds all data object instances LeopphEngine keeps track of.
	// Its purpose is to manage and centralize access to data that need to be shared, or accessed by multiple systems.
	class DataManager final
	{
		public:
			[[nodiscard]] static auto Instance() -> DataManager&;

			// Destroys everything.
			auto Clear() -> void;

			// Stores a non-owning pointer to the texture instance.
			// The function does NOT check for duplicates.
			auto RegisterTexture(Texture* texture) -> void;

			// Removes all pointers that have that point to the same address as the passed pointer from the registry.
			auto UnregisterTexture(Texture* texture) -> void;

			// Returns a shared_ptr to the Texture that was loaded from the passed path, or, if not found, nullptr.
			[[nodiscard]] auto FindTexture(const std::filesystem::path& path) -> std::shared_ptr<Texture>;

			// If present, returns a pointer to the SkyboxImpl that was loaded from the passed path.
			// If not found, creates a new instance and returns a pointer to that.
			[[nodiscard]] auto CreateOrGetSkyboxImpl(std::filesystem::path allPaths) -> SkyboxImpl*;

			// Returns the number of Skybox instances pointing to the SkyboxImpl instance pointed to by the passed pointer.
			// The function does NOT check whether the instance is actually registered or not.
			[[nodiscard]] auto SkyboxHandleCount(const SkyboxImpl* skybox) const -> std::size_t;

			// Unregisters all Skybox handles and destroys the impl instance.
			// The function does NOT check whether the passed pointer points to a stored instance or not.
			auto DestroySkyboxImpl(const SkyboxImpl* skybox) -> void;

			// Stores the handle pointer for the passed SkyboxImpl instance.
			// The function does NOT check for duplicates or if the passed impl instance is even registered.
			auto RegisterSkyboxHandle(const SkyboxImpl* skybox, Skybox* handle) -> void;

			// Removes all handle pointers from the passed SkyboxImpl instance that point to the same handle as the passed pointer.
			// The function does NOT check whether the passed SkyboxImpl instance is registered or not.
			auto UnregisterSkyboxHandle(const SkyboxImpl* skybox, Skybox* handle) -> void;

			// Takes ownership of the Entity object and stores it.
			// The function does NOT check if the Entity is already pointed to by a stored unique_ptr.
			auto StoreEntity(std::unique_ptr<Entity> entity) -> void;

			// Removes and destroys all unique_ptrs that point to the same Entity as the passed pointer.
			// Removes and destroys the associated Components.
			auto DestroyEntity(const Entity* entity) -> void;

			// Returns a pointer to the stored Entity with the passed name, or nullptr.
			[[nodiscard]] auto FindEntity(const std::string& name) -> Entity*;

			// Returns a collection of the Components that are attached to the Entity pointed to by the passed pointer.
			// The function does NOT check whether the Entity is registered or not.
			[[nodiscard]] auto ComponentsOfEntity(const Entity* entity) const -> const std::vector<std::unique_ptr<Component>>&;

			// Adds the Component to the collection of Components for the Component's Entity.
			// The function does NOT check for duplicates or if the Entity is even registered.
			auto RegisterComponentForEntity(std::unique_ptr<Component>&& component) -> void;

			// Removes and destroys all Components and pointers that have the same value as the passed Component pointer.
			// The function does NOT check whether the Component or its Entity are registered or not.
			auto UnregisterComponentFromEntity(const Component* component) -> void;

			// Stores the passed pointer in the collection of Behaviors.
			// The function does NOT check for duplicates.
			auto RegisterBehavior(Behavior* behavior) -> void;

			// Removes all registered Behavior pointers that point to the same Behavior instance as the passed pointer.
			auto UnregisterBehavior(const Behavior* behavior) -> void;

			// Stores the passed pointer in the collection of SpotLights.
			// The function does NOT check for duplicates.
			auto RegisterSpotLight(const SpotLight* spotLight) -> void;

			// Removes all registered SpotLight pointers that point to the same SpotLight instance as the passed pointer.
			auto UnregisterSpotLight(const SpotLight* spotLight) -> void;

			// Stores the passed pointer in the collection of PointLights.
			// The function does NOT check for duplicates.
			auto RegisterPointLight(const PointLight* pointLight) -> void;

			// Removes all registered PointLight pointers that point to the same PointLight instance as the passed pointer.
			auto UnregisterPointLight(const PointLight* pointLight) -> void;

			// Returns a shared_ptr to an already existing registered MeshDataGroup instance whose ID is equal to the passed ID.
			// If not found, returns nullptr.
			[[nodiscard]] auto FindMeshDataGroup(const std::string& id) -> std::shared_ptr<MeshDataGroup>;

			// Stores the passed pointer in the collection of MeshDataGroups.
			// The function does NOT check for duplicates.
			auto RegisterMeshDataGroup(MeshDataGroup* meshData) -> void;

			// Removes all registered MeshDataGroup pointers the point to the same MeshDataGroup instance as the passed pointer.
			auto UnregisterMeshDataGroup(MeshDataGroup* meshData) -> void;

			// Returns a pointer to an already registered GlMeshGroup instance.
			// If not present, creates a new one and returns a pointer to that.
			[[nodiscard]] auto CreateOrGetMeshGroup(std::shared_ptr<const MeshDataGroup>&& meshDataGroup) -> const GlMeshGroup*;

			// Returns the number of instances that are registered for the MeshGroup.
			// The function does NOT check whether the passed MeshGroup is registered or not.
			[[nodiscard]] auto MeshGroupInstanceCount(const GlMeshGroup* meshGroup) const -> std::size_t;

			// Unregisters all instances and destroys the MeshGroup.
			// The function does NOT check whether the passed MeshGroup is registered or not.
			auto DestroyMeshGroup(const GlMeshGroup* meshGroup) -> void;

			// Stores the passed instance pointer for the MeshGroup pointed to by the passed pointer.
			// The function does NOT check for duplicated or if the passed MeshGroup is even registered.
			auto RegisterInstanceForMeshGroup(const GlMeshGroup* meshGroup, const RenderComponent* instance) -> void;

			/* Removes all instance pointers that point to the same instance as the passed pointer from the passed MeshGroup.
			 * The function does NOT check whether the passed MeshGroup is registered or not. */
			auto UnregisterInstanceFromMeshGroup(const GlMeshGroup* meshGroup, const RenderComponent* instance) -> void;

			[[nodiscard]] constexpr auto Behaviors() const noexcept -> auto&;
			[[nodiscard]] constexpr auto DirectionalLight() const noexcept;
			[[nodiscard]] constexpr auto SpotLights() const noexcept -> auto&;
			[[nodiscard]] constexpr auto PointLights() const noexcept -> auto&;
			[[nodiscard]] constexpr auto MeshGroupsAndInstances() const noexcept -> auto&;

			constexpr auto DirectionalLight(const leopph::DirectionalLight* dirLight) noexcept;

		private:
			struct EntityAndComponents
			{
				// Owning pointer to Entity
				std::unique_ptr<Entity> Entity;
				// Owning pointers to Components
				std::vector<std::unique_ptr<Component>> Components;
			};


			struct MeshGroupAndInstances
			{
				// Owning pointer to MeshGroup
				std::unique_ptr<GlMeshGroup> MeshGroup;
				// Non-owning pointers to instances
				std::vector<const RenderComponent*> Instances;
			};


			// Non-owning pointers to all MeshDataGroup instances.
			std::vector<MeshDataGroup*> m_MeshData;

			// Non-owning pointers to all Texture instances.
			std::vector<Texture*> m_Textures;

			// Non-owning pointer to the lastly created DirectionalLight.
			const leopph::DirectionalLight* m_DirLight{nullptr};

			// Non-owning pointers to all SpotLights.
			std::vector<const SpotLight*> m_SpotLights;

			// Non-owning pointers to all PointLights.
			std::vector<const PointLight*> m_PointLights;

			// Non-owning pointers to all Behaviors.
			std::vector<Behavior*> m_Behaviors;

			// SkyboxImpl instances and non-owning pointer to all the Skybox handles pointing to them.
			std::unordered_map<SkyboxImpl, std::vector<Skybox*>, PathedHash<SkyboxImpl>, PathedEqual<SkyboxImpl>> m_Skyboxes;

			// All MeshGroups and all of their registered instances
			std::vector<MeshGroupAndInstances> m_Renderables;

			// All Entities and all of their attached Components
			std::vector<EntityAndComponents> m_EntitiesAndComponents;

			auto SortTextures() -> void;
			auto SortEntities() -> void;
			auto SortMeshData() -> void;

			// Returns a non-const iterator to the element or past-the-end.
			[[nodiscard]] auto FindEntityInternal(const std::string& name) -> decltype(m_EntitiesAndComponents)::iterator;
			// Returns a const iterator to the element or past-the-end.
			[[nodiscard]] auto FindEntityInternal(const std::string& name) const -> decltype(m_EntitiesAndComponents)::const_iterator;
			// Helper function to get const and non-const iterators depending on context.
			[[nodiscard]] static auto FindEntityInternalCommon(auto* self, const std::string& name) -> decltype(auto);

			// Returns a non-const iterator to the element or past-the-end.
			[[nodiscard]] auto FindMeshGroupInternal(const GlMeshGroup* meshGroup) -> decltype(m_Renderables)::iterator;
			// Returns a const iterator to the element or past-the-end.
			[[nodiscard]] auto FindMeshGroupInternal(const GlMeshGroup* meshGroup) const -> decltype(m_Renderables)::const_iterator;
			// Helper function to get const and non-const iterators depending on context.
			[[nodiscard]] static auto FindMeshGroupInternalCommon(auto* self, const GlMeshGroup* meshGroup) -> decltype(auto);
	};


	[[nodiscard]] constexpr auto DataManager::Behaviors() const noexcept -> auto&
	{
		return m_Behaviors;
	}


	[[nodiscard]] constexpr auto DataManager::DirectionalLight() const noexcept
	{
		return m_DirLight;
	}


	[[nodiscard]] constexpr auto DataManager::SpotLights() const noexcept -> auto&
	{
		return m_SpotLights;
	}


	[[nodiscard]] constexpr auto DataManager::PointLights() const noexcept -> auto&
	{
		return m_PointLights;
	}


	[[nodiscard]] constexpr auto DataManager::MeshGroupsAndInstances() const noexcept -> auto&
	{
		return m_Renderables;
	}


	constexpr auto DataManager::DirectionalLight(const leopph::DirectionalLight* const dirLight) noexcept
	{
		m_DirLight = dirLight;
	}


	[[nodiscard]] auto DataManager::FindEntityInternalCommon(auto* const self, const std::string& name) -> decltype(auto)
	{
		auto it{
			std::ranges::lower_bound(self->m_EntitiesAndComponents, name, [](const auto& elemName, const auto& value)
			                         {
				                         return elemName < value;
			                         }, [](const auto& elem) -> const auto&
			                         {
				                         return elem.Entity->Name();
			                         })
		};
		if (it != self->m_EntitiesAndComponents.end() && it->Entity->Name() == name)
		{
			return it;
		}
		return decltype(it){self->m_EntitiesAndComponents.end()};
	}


	auto DataManager::FindMeshGroupInternalCommon(auto* const self, const GlMeshGroup* const meshGroup) -> decltype(auto)
	{
		return std::ranges::find_if(self->m_Renderables, [&](const auto& elem)
		{
			return elem.MeshGroup.get() == meshGroup;
		});
	}
}
