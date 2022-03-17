#pragma once

#include "Poelo.hpp"
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
#include "../util/less/PoeloLess.hpp"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <set>
#include <span>
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
			[[nodiscard]] static
			auto Instance() -> DataManager&;

			// Destroys everything.
			auto Clear() -> void;

			// Takes ownership of the Poelo instance.
			auto Store(std::unique_ptr<Poelo> poelo) -> void;

			// Destroys the passed Poelo instance if found.
			// Returns whether deletion took place.
			auto Destroy(const Poelo* poelo) -> bool;

			// Non-owningly registers an Entity instance.
			// The function does NOT check if the Entity instance is already registered.
			auto RegisterEntity(Entity* entity) -> void;

			// Unregisters the passed Entity instance.
			// Removes and destroys the attached Components.
			auto UnregisterEntity(const Entity* entity) -> void;

			// Returns a pointer to the stored Entity with the passed name, or nullptr.
			[[nodiscard]]
			auto FindEntity(const std::string& name) -> Entity*;

			// Adds the Component to the Entity's collection of active/inactive Components depending on the value of active.
			auto RegisterComponentForEntity(const Entity* entity, Component* component, bool active) -> void;

			// Removes the Component from the Entity's collection of active/inactive Components depending on the value of active.
			auto UnregisterComponentFromEntity(const Entity* entity, Component* component, bool active) -> void;

			// Returns the Entity's collection of active/inactive Components depending on the value of active.
			[[nodiscard]]
			auto ComponentsOfEntity(const Entity* entity, bool active) const -> std::span<Component* const>;

			// Adds the Behavior to the collection of Behaviors depending on the value of active.
			// Does NOT check for duplicates.
			auto RegisterBehavior(Behavior* behavior, bool active) -> void;

			// Removes the Behavior from the collection of Behaviors depending on the value of active.
			// Removes duplicates.
			auto UnregisterBehavior(const Behavior* behavior, bool active) -> void;

			// Returns the registered, currently active Behaviors.
			[[nodiscard]] constexpr
			auto ActiveBehaviors() const noexcept -> auto&;

			// Adds the DirLight to the collection of DirLights depending on the value of active.
			// Does NOT check for duplicates.
			auto RegisterDirLight(const DirectionalLight* dirLight, bool active) -> void;

			// Removes the DirLight from the collection of DirLights depending on the value of active.
			// Remove duplicates.
			auto UnregisterDirLight(const DirectionalLight* dirLight, bool active) -> void;

			// Returns the last created active DirectionalLight, or nullptr if none.
			[[nodiscard]]
			auto DirectionalLight() const -> const DirectionalLight*;

			// Adds the SpotLight to the collection of SpotLights depending on the value of active.
			// Does NOT check for duplicates.
			auto RegisterSpotLight(const SpotLight* spotLight, bool active) -> void;

			// Removes the SpotLight from the collection of SpotLights depending on the value of active.
			// Removes duplicates.
			auto UnregisterSpotLight(const SpotLight* spotLight, bool active) -> void;

			// Returns the registered, currently active SpotLights.
			[[nodiscard]] constexpr
			auto ActiveSpotLights() const noexcept -> auto&;

			// Adds the PointLight to the collection of PointLights depending on the value of active.
			// Does NOT check for duplicates.
			auto RegisterPointLight(const PointLight* pointLight, bool active) -> void;

			// Removes the PointLight from the collection of PointLights depending on the value of active.
			// Removes duplicates.
			auto UnregisterPointLight(const PointLight* pointLight, bool active) -> void;

			// Returns the registered, currently active PointLights.
			[[nodiscard]] constexpr
			auto ActivePointLights() const noexcept -> auto&;

			// Stores the passed pointer in the collection of MeshDataGroups.
			// The function does NOT check for duplicates.
			auto RegisterMeshDataGroup(MeshDataGroup* meshData) -> void;

			// Removes all registered MeshDataGroup pointers the point to the same MeshDataGroup instance as the passed pointer.
			auto UnregisterMeshDataGroup(MeshDataGroup* meshData) -> void;

			// Returns a shared_ptr to an already existing registered MeshDataGroup instance whose ID is equal to the passed ID.
			// If not found, returns nullptr.
			[[nodiscard]]
			auto FindMeshDataGroup(const std::string& id) -> std::shared_ptr<MeshDataGroup>;

			// Adds the GlMeshGroup to the collection of GlMeshGroups.
			// Does NOT check for duplicates.
			auto RegisterGlMeshGroup(GlMeshGroup* glMeshGroup) -> void;

			// Removes the GlMeshGroup from the collection of GlMeshGroups.
			// Removes duplicates.
			auto UnregisterGlMeshGroup(const GlMeshGroup* glMeshGroup) -> void;

			// Returns a shared_ptr to the GlMeshGroup using the MeshDataGroup, or nullptr if not found.
			[[nodiscard]]
			auto FindGlMeshGroup(const MeshDataGroup* meshDataGroup) -> std::shared_ptr<GlMeshGroup>;

			// Adds the RenderComponent to the GlMeshGroups's collection of instances depending on the value of active.
			// Does NOT check for duplicates.
			auto RegisterInstanceForMeshGroup(const GlMeshGroup* meshGroup, const RenderComponent* instance, bool active) -> void;

			// Removes the RenderComponent from the GlMeshGroup's collection of instances depending on the value of active.
			// Removes duplicats.
			auto UnregisterInstanceFromMeshGroup(const GlMeshGroup* meshGroup, const RenderComponent* instance, bool active) -> void;

			// Returns the number of instances that are registered for the MeshGroup.
			// The function does NOT check whether the passed MeshGroup is registered or not.
			[[nodiscard]]
			auto MeshGroupInstanceCount(const GlMeshGroup* meshGroup) const -> std::size_t;

			// Returns all registered GlMeshGroups and their active and inactive instances.
			[[nodiscard]] constexpr
			auto MeshGroupsAndInstances() const noexcept -> auto&;

			// If present, returns a pointer to the SkyboxImpl that was loaded from the passed path.
			// If not found, creates a new instance and returns a pointer to that.
			[[nodiscard]]
			auto CreateOrGetSkyboxImpl(std::filesystem::path allPaths) -> SkyboxImpl*;

			// Unregisters all Skybox handles and destroys the impl instance.
			// The function does NOT check whether the passed pointer points to a stored instance or not.
			auto DestroySkyboxImpl(const SkyboxImpl* skybox) -> void;

			// Stores the handle pointer for the passed SkyboxImpl instance.
			// The function does NOT check for duplicates or if the passed impl instance is even registered.
			auto RegisterSkyboxHandle(const SkyboxImpl* skybox, Skybox* handle) -> void;

			// Removes all handle pointers from the passed SkyboxImpl instance that point to the same handle as the passed pointer.
			// The function does NOT check whether the passed SkyboxImpl instance is registered or not.
			auto UnregisterSkyboxHandle(const SkyboxImpl* skybox, Skybox* handle) -> void;

			// Returns the number of Skybox instances pointing to the SkyboxImpl instance pointed to by the passed pointer.
			// The function does NOT check whether the instance is actually registered or not.
			[[nodiscard]]
			auto SkyboxHandleCount(const SkyboxImpl* skybox) const -> std::size_t;

			// Stores a non-owning pointer to the texture instance.
			// The function does NOT check for duplicates.
			auto RegisterTexture(Texture* texture) -> void;

			// Removes all pointers that have that point to the same address as the passed pointer from the registry.
			auto UnregisterTexture(Texture* texture) -> void;

			// Returns a shared_ptr to the Texture that was loaded from the passed path, or, if not found, nullptr.
			[[nodiscard]]
			auto FindTexture(const std::filesystem::path& path) -> std::shared_ptr<Texture>;

		private:
			struct EntityAndComponents
			{
				// Non-owning pointer to Entity
				Entity* Entity;
				// Owning pointers to active Components
				std::vector<Component*> ActiveComponents;
				// Owning pointers to inactive Components
				std::vector<Component*> InactiveComponents;
			};


			struct MeshGroupAndInstances
			{
				// Non-owning pointer to MeshGroup
				GlMeshGroup* MeshGroup;
				// Non-owning pointers to active instances
				std::vector<const RenderComponent*> ActiveInstances;
				// Non-owning pointers to inactive instances
				std::vector<const RenderComponent*> InactiveInstances;
			};


			auto SortEntities() -> void;
			auto SortMeshData() -> void;
			auto SortTextures() -> void;

			// All engine-owned objects.
			std::set<std::unique_ptr<Poelo>, PoeloLess> m_Poelos;

			// Non-owning pointers to all MeshDataGroup instances.
			std::vector<MeshDataGroup*> m_MeshData;

			// Non-owning pointers to all Texture instances.
			std::vector<Texture*> m_Textures;

			// Non-owning pointers to the active DirectionalLights.
			std::vector<const leopph::DirectionalLight*> m_ActiveDirLights;
			// Non-owning pointers to the inactive DirectionalLights.
			std::vector<const leopph::DirectionalLight*> m_InactiveDirLights;

			// Non-owning pointers to all active SpotLights.
			std::vector<const SpotLight*> m_ActiveSpotLights;
			// Non-owning pointers to all inactive SpotLights.
			std::vector<const SpotLight*> m_InactiveSpotLights;

			// Non-owning pointers to all active PointLights.
			std::vector<const PointLight*> m_ActivePointLights;
			// Non-owning pointers to all inactive PointLights.
			std::vector<const PointLight*> m_InactivePointLights;

			// Non-owning pointers to active Behaviors.
			std::vector<Behavior*> m_ActiveBehaviors;
			// Non-owning pointers to inactive Behaviors.
			std::vector<Behavior*> m_InactiveBehaviors;

			// SkyboxImpl instances and non-owning pointer to all the Skybox handles pointing to them.
			std::unordered_map<SkyboxImpl, std::vector<Skybox*>, PathedHash<SkyboxImpl>, PathedEqual<SkyboxImpl>> m_Skyboxes;

			// All MeshGroups and all of their registered instances
			std::vector<MeshGroupAndInstances> m_Renderables;

			// All Entities and all of their attached Components
			std::vector<EntityAndComponents> m_EntitiesAndComponents;

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


	[[nodiscard]] constexpr auto DataManager::ActiveBehaviors() const noexcept -> auto&
	{
		return m_ActiveBehaviors;
	}


	[[nodiscard]] constexpr auto DataManager::ActiveSpotLights() const noexcept -> auto&
	{
		return m_ActiveSpotLights;
	}


	[[nodiscard]] constexpr auto DataManager::ActivePointLights() const noexcept -> auto&
	{
		return m_ActivePointLights;
	}


	[[nodiscard]] constexpr auto DataManager::MeshGroupsAndInstances() const noexcept -> auto&
	{
		return m_Renderables;
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
		return std::ranges::find_if(self->m_Renderables, [meshGroup](const auto& elem)
		{
			return elem.MeshGroup == meshGroup;
		});
	}
}
