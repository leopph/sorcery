#pragma once

#include "PoeloBase.hpp"
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
#include "../util/less/PoeloBaseLess.hpp"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <set>
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

			// Takes ownership of the PoeloBase instance.
			auto Store(std::unique_ptr<PoeloBase> poelo) -> void;
			// Destroys the passed PoeloBase instance if found.
			// Returns whether deletion took place.
			auto Destroy(const PoeloBase* poelo) -> bool;

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

			// Non-owningly registers an Entity instance.
			// The function does NOT check if the Entity instance is already registered.
			auto RegisterEntity(Entity* entity) -> void;

			// Unregisters the passed Entity instance.
			// Removes and destroys the attached Components.
			auto UnregisterEntity(const Entity* entity) -> void;

			// Returns a pointer to the stored Entity with the passed name, or nullptr.
			[[nodiscard]] auto FindEntity(const std::string& name) -> Entity*;

			// Returns a collection of active Components that are attached to the Entity pointed to by the passed pointer.
			// Does NOT check whether the Entity is registered or not.
			[[nodiscard]] auto ActiveComponentsOfEntity(const Entity* entity) const -> const std::vector<std::unique_ptr<Component>>&;
			// Returns a collection of inactive Components that are attached to the Entity pointed to by the passed pointer.
			// Does NOT check whether the Entity is registered or not.
			[[nodiscard]] auto InactiveComponentsOfEntity(const Entity* entity) const -> const std::vector<std::unique_ptr<Component>>&;

			// Adds the Component to the collection of active Components for the Component's Entity.
			// Does NOT check for duplicates or whether the Entity is registered.
			auto RegisterActiveComponentForEntity(std::unique_ptr<Component>&& component) -> void;
			// Adds the Component to the collection of inactive Components for the Component's Entity.
			// Does NOT check for duplicates or whether the Entity is registered.
			auto RegisterInactiveComponentForEntity(std::unique_ptr<Component>&& component) -> void;

			// Removes the passed component from the its Entity's collection of active Components.
			// Returns the Component, or null if not found.
			// Does NOT check whether the Entity is registered.
			auto UnregisterActiveComponentFromEntity(const Component* component) -> std::unique_ptr<Component>;
			// Removes the passed component from the its Entity's collection of inactive Components.
			// Returns the Component, or null if not found.
			// Does NOT check whether the Entity is registered.
			auto UnregisterInactiveComponentFromEntity(const Component* component) -> std::unique_ptr<Component>;

			// Stores the passed pointer in the collection of active Behaviors.
			// The function does NOT check for duplicates.
			auto RegisterActiveBehavior(Behavior* behavior) -> void;
			// Stores the passed pointer in the collection of inactive Behaviors.
			// The function does NOT check for duplicates.
			auto RegisterInactiveBehavior(Behavior* behavior) -> void;

			// Removes the passed Behavior from the collection of active Behaviors.
			auto UnregisterActiveBehavior(const Behavior* behavior) -> void;
			// Removes the passed Behavior from the collection of inactive Behaviors.
			auto UnregisterInactiveBehavior(const Behavior* behavior) -> void;

			// Stores the passed pointer in the collection of active SpotLights.
			// Does NOT check for duplicates.
			auto RegisterActiveSpotLight(const SpotLight* spotLight) -> void;
			// Stores the passed pointer in the collection of inactive SpotLights.
			// Does NOT check for duplicates.
			auto RegisterInactiveSpotLight(const SpotLight* spotLight) -> void;

			// Removes the passed SpotLight from the collection of active SpotLights.
			auto UnregisterActiveSpotLight(const SpotLight* spotLight) -> void;
			// Removes the passed SpotLight from the collection of inactive SpotLights.
			auto UnregisterInactiveSpotLight(const SpotLight* spotLight) -> void;

			// Stores the passed pointer in the collection of active PointLights.
			// Does NOT check for duplicates.
			auto RegisterActivePointLight(const PointLight* pointLight) -> void;
			// Stores the passed pointer in the collection of inactive PointLights.
			// Does NOT check for duplicates.
			auto RegisterInactivePointLight(const PointLight* pointLight) -> void;

			// Removes the passed SpotLight from the collection of active PointLights.
			auto UnregisterActivePointLight(const PointLight* pointLight) -> void;
			// Removes the passed SpotLight from the collection of inactive PointLights.
			auto UnregisterInactivePointLight(const PointLight* pointLight) -> void;

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

			// Stores the passed instance for the passed MeshGroup's collection of active instances.
			// Does NOT check for duplicated or whether the MeshGroup is registered.
			auto RegisterActiveInstanceForMeshGroup(const GlMeshGroup* meshGroup, const RenderComponent* instance) -> void;
			// Stores the passed instance for the passed MeshGroup's collection of inactive instances.
			// Does NOT check for duplicated or whether the MeshGroup is registered.
			auto RegisterInactiveInstanceForMeshGroup(const GlMeshGroup* meshGroup, const RenderComponent* instance) -> void;

			// Removes the instance from the passed MeshGroup's collection of active instances.
			// Does NOT check whether the passed MeshGroup is registered or not.
			auto UnregisterActiveInstanceFromMeshGroup(const GlMeshGroup* meshGroup, const RenderComponent* instance) -> void;
			// Removes the instance from the passed MeshGroup's collection of inactive instances.
			// Does NOT check whether the passed MeshGroup is registered or not.
			auto UnregisterInactiveInstanceFromMeshGroup(const GlMeshGroup* meshGroup, const RenderComponent* instance) -> void;

			// Adds the DirLight to the collection of active DirLights.
			// Does NOT check for duplicates.
			auto RegisterActiveDirLight(const DirectionalLight* dirLight) -> void;
			// Adds the DirLight to the collection of inactive DirLights.
			// Does NOT check for duplicates.
			auto RegisterInactiveDirLight(const DirectionalLight* dirLight) -> void;

			// Removes the DirLight from the collection of active DirLights.
			auto UnregisterActiveDirLight(const DirectionalLight* dirLight) -> void;
			// Removes the DirLight from the collection of inactive DirLights.
			auto UnregisterInactiveDirLight(const DirectionalLight* dirLight) -> void;

			[[nodiscard]] constexpr auto ActiveBehaviors() const noexcept -> auto&;
			// Returns the last created active DirectionalLight, or nullptr if none.
			[[nodiscard]] auto DirectionalLight() const -> const DirectionalLight*;
			[[nodiscard]] constexpr auto ActiveSpotLights() const noexcept -> auto&;
			[[nodiscard]] constexpr auto ActivePointLights() const noexcept -> auto&;
			[[nodiscard]] constexpr auto MeshGroupsAndInstances() const noexcept -> auto&;

		private:
			struct EntityAndComponents
			{
				// Owning pointer to Entity
				Entity* Entity;
				// Owning pointers to active Components
				std::vector<std::unique_ptr<Component>> ActiveComponents;
				// Owning pointers to inactive Components
				std::vector<std::unique_ptr<Component>> InactiveComponents;
			};


			struct MeshGroupAndInstances
			{
				// Owning pointer to MeshGroup
				std::unique_ptr<GlMeshGroup> MeshGroup;
				// Non-owning pointers to active instances
				std::vector<const RenderComponent*> ActiveInstances;
				// Non-owning pointers to inactive instances
				std::vector<const RenderComponent*> InactiveInstances;
			};


			// All engine-owned objects.
			std::set<std::unique_ptr<PoeloBase>, PoeloBaseLess> m_Poelos;


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

			// Find, erases, and returns the specified component from the passed vector.
			// Returns nullptr if not found.
			[[nodiscard]] static auto EraseComponentInternal(std::vector<std::unique_ptr<Component>>& components, const Component* component) -> std::unique_ptr<Component>;
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
		return std::ranges::find_if(self->m_Renderables, [&](const auto& elem)
		{
			return elem.MeshGroup.get() == meshGroup;
		});
	}
}
