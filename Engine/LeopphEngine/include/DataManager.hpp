#pragma once

#include "Behavior.hpp"
#include "Component.hpp"
#include "DirLight.hpp"
#include "Entity.hpp"
#include "Poelo.hpp"
#include "PointLight.hpp"
#include "RenderComponent.hpp"
#include "Skybox.hpp"
#include "SpotLight.hpp"
#include "rendering/SkyboxImpl.hpp"
#include "rendering/GlMeshGroup.hpp"
#include "util/equal/PathedEqual.hpp"
#include "util/hash/PathedHash.hpp"
#include "util/less/PoeloLess.hpp"

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
			auto Destroy(Poelo const* poelo) -> bool;

			// Non-owningly registers an Entity instance.
			// The function does NOT check if the Entity instance is already registered.
			auto RegisterEntity(Entity* entity) -> void;

			// Unregisters the passed Entity instance.
			// Removes and destroys the attached Components.
			auto UnregisterEntity(Entity const* entity) -> void;

			// Returns a pointer to the stored Entity with the passed name, or nullptr.
			[[nodiscard]]
			auto FindEntity(std::string const& name) -> Entity*;

			// Adds the Component to the Entity's collection of active/inactive Components depending on the value of active.
			auto RegisterComponent(Entity const* entity, ComponentPtr<> component, bool active) -> void;

			// Removes the Component from the Entity's collection of active/inactive Components depending on the value of active.
			// Returns the removed pointer.
			auto UnregisterComponent(Entity const* entity, Component const* component, bool active) -> ComponentPtr<>;

			// Returns the Entity's collection of active/inactive Components depending on the value of active.
			[[nodiscard]]
			auto ComponentsOfEntity(Entity const* entity, bool active) const -> std::span<ComponentPtr<> const>;

			// Adds the Behavior to the collection of active Behaviors.
			// Does NOT check for duplicates.
			auto RegisterActiveBehavior(Behavior* behavior) -> void;

			// Removes the Behavior from the collection of active Behaviors.
			// Removes duplicates.
			// Not registered behaviors are ignored.
			auto UnregisterActiveBehavior(Behavior const* behavior) -> void;

			// Returns the registered, currently active Behaviors.
			[[nodiscard]] constexpr
			auto ActiveBehaviors() const noexcept -> auto&;

			// Adds the DirLight to the collection of active DirLights.
			// Does NOT check for duplicates.
			auto RegisterActiveDirLight(DirectionalLight const* dirLight) -> void;

			// Removes the DirLight from the collection of active DirLights.
			// Remove duplicates.
			// Not registered DirLights are ignored.
			auto UnregisterActiveDirLight(DirectionalLight const* dirLight) -> void;

			// Returns the last created active DirectionalLight, or nullptr if none.
			[[nodiscard]]
			auto DirectionalLight() const -> DirectionalLight const*;

			// Adds the SpotLight to the collection of active SpotLights.
			// Does NOT check for duplicates.
			auto RegisterActiveSpotLight(SpotLight const* spotLight) -> void;

			// Removes the SpotLight from the collection of active SpotLights.
			// Removes duplicates.
			// Not registered SpotLights are ignored.
			auto UnregisterActiveSpotLight(SpotLight const* spotLight) -> void;

			// Returns the registered, currently active SpotLights.
			[[nodiscard]] constexpr
			auto ActiveSpotLights() const noexcept -> auto&;

			// Adds the PointLight to the collection of active PointLights.
			// Does NOT check for duplicates.
			auto RegisterActivePointLight(PointLight const* pointLight) -> void;

			// Removes the PointLight from the collection of active PointLights.
			// Removes duplicates.
			// Not registered PointLights are ignored.
			auto UnregisterActivePointLight(PointLight const* pointLight) -> void;

			// Returns the registered, currently active PointLights.
			[[nodiscard]] constexpr
			auto ActivePointLights() const noexcept -> auto&;

			// Duplicates are ignored.
			auto RegisterGlMeshGroup(GlMeshGroup* glMeshGroup) -> void;

			// Also unregisters the associated active RenderComponents.
			auto UnregisterGlMeshGroup(GlMeshGroup* glMeshGroup) -> void;

			// Adds the RenderComponent to the GlMeshGroups's collection of active RenderComponents.
			// Does NOT check for duplicates.
			auto RegisterActiveRenderComponent(std::shared_ptr<GlMeshGroup> const& glMeshGroup, RenderComponent* renderComponent) -> void;

			// Removes the RenderComponent from the GlMeshGroup's collection of active RenderComponents.
			// Removes duplicates.
			// Not registered RenderComponents are ignored.
			auto UnregisterActiveRenderComponent(std::shared_ptr<GlMeshGroup> const& glMeshGroup, RenderComponent const* renderComponent) -> void;

			// Returns the number of active RenderComponents that are registered for the GlMeshGroup.
			[[nodiscard]]
			auto RenderComponentCount(GlMeshGroup* glMeshGroup) const -> std::size_t;

			// Returns all registered GlMeshGroups and their active instances.
			[[nodiscard]] constexpr
			auto MeshGroupsAndActiveInstances() const noexcept -> auto&;

			// If present, returns a pointer to the SkyboxImpl that was loaded from the passed path.
			// If not found, creates a new instance and returns a pointer to that.
			[[nodiscard]]
			auto CreateOrGetSkyboxImpl(std::filesystem::path allPaths) -> SkyboxImpl*;

			// Unregisters all Skybox handles and destroys the impl instance.
			// The function does NOT check whether the passed pointer points to a stored instance or not.
			auto DestroySkyboxImpl(SkyboxImpl const* skybox) -> void;

			// Stores the handle pointer for the passed SkyboxImpl instance.
			// The function does NOT check for duplicates or if the passed impl instance is even registered.
			auto RegisterSkyboxHandle(SkyboxImpl const* skybox, Skybox* handle) -> void;

			// Removes all handle pointers from the passed SkyboxImpl instance that point to the same handle as the passed pointer.
			// The function does NOT check whether the passed SkyboxImpl instance is registered or not.
			auto UnregisterSkyboxHandle(SkyboxImpl const* skybox, Skybox* handle) -> void;

			// Returns the number of Skybox instances pointing to the SkyboxImpl instance pointed to by the passed pointer.
			// The function does NOT check whether the instance is actually registered or not.
			[[nodiscard]]
			auto SkyboxHandleCount(SkyboxImpl const* skybox) const -> std::size_t;

		private:
			struct EntityEntry
			{
				Entity* Entity;
				std::vector<ComponentPtr<>> ActiveComponents;
				std::vector<ComponentPtr<>> InactiveComponents;
			};


			using EntityOrderFunc = std::ranges::less;
			using BehaviorOrderFunc = std::ranges::less;

			auto SortEntities() -> void;

		public:
			auto SortActiveBehaviors() -> void;

		private:
			// All engine-owned objects.
			std::set<std::unique_ptr<Poelo>, PoeloLess> m_Poelos;

			// All Entities and all of their attached Components
			std::vector<EntityEntry> m_EntitiesAndComponents;

			// Non-owning pointers to active Behaviors.
			std::vector<Behavior*> m_ActiveBehaviors;

			// Non-owning pointers to the active DirectionalLights.
			std::vector<leopph::DirectionalLight const*> m_ActiveDirLights;

			// Non-owning pointers to all active SpotLights.
			std::vector<SpotLight const*> m_ActiveSpotLights;

			// Non-owning pointers to all active PointLights.
			std::vector<PointLight const*> m_ActivePointLights;

			// SkyboxImpl instances and non-owning pointer to all the Skybox handles pointing to them.
			std::unordered_map<SkyboxImpl, std::vector<Skybox*>, PathedHash<SkyboxImpl>, PathedEqual<SkyboxImpl>> m_Skyboxes;

			// Registered GlMeshGroups and their associated active RenderComponents.
			std::unordered_map<GlMeshGroup*, std::vector<RenderComponent*>> m_Renderables;

			// Returns a non-const iterator to the element or past-the-end.
			[[nodiscard]]
			auto GetEntityIterator(std::string const& name) -> decltype(m_EntitiesAndComponents)::iterator;

			// Returns a const iterator to the element or past-the-end.
			[[nodiscard]]
			auto GetEntityIterator(std::string const& name) const -> decltype(m_EntitiesAndComponents)::const_iterator;

			// Helper function to get const and non-const iterators depending on context.
			[[nodiscard]] static
			auto GetEntityIteratorCommon(auto* self, std::string const& name) -> decltype(auto);
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


	[[nodiscard]] constexpr auto DataManager::MeshGroupsAndActiveInstances() const noexcept -> auto&
	{
		return m_Renderables;
	}


	[[nodiscard]] auto DataManager::GetEntityIteratorCommon(auto* const self, std::string const& name) -> decltype(auto)
	{
		auto it{
			std::ranges::lower_bound(self->m_EntitiesAndComponents, name, [](auto const& elemName, auto const& value)
			                         {
				                         return elemName < value;
			                         }, [](auto const& elem) -> auto const&
			                         {
				                         return elem.Entity->Name();
			                         })
		};
		if (it != self->m_EntitiesAndComponents.end() && *it->Entity == name)
		{
			return it;
		}
		return decltype(it){self->m_EntitiesAndComponents.end()};
	}
}
