#pragma once

#include "Behavior.hpp"
#include "Component.hpp"
#include "DirLight.hpp"
#include "Entity.hpp"
#include "Poelo.hpp"
#include "PointLight.hpp"
#include "SpotLight.hpp"
#include "util/less/PoeloLess.hpp"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <set>
#include <span>
#include <string>
#include <vector>


namespace leopph::internal
{
	// A DataManager object holds all data object instances LeopphEngine keeps track of.
	// Its purpose is to manage and centralize access to data that need to be shared, or accessed by multiple systems.
	class DataManager final
	{
		public:
			DataManager() = default;

			DataManager(DataManager const& other) = delete;
			auto operator=(DataManager const& other) -> void = delete;

			DataManager(DataManager&& other) = delete;
			auto operator=(DataManager&& other) -> void = delete;

			~DataManager();

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
			[[nodiscard]] auto FindEntity(std::string const& name) -> Entity*;

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
			[[nodiscard]] auto ActiveBehaviors() const noexcept -> std::span<Behavior* const>;

			// Adds the DirLight to the collection of active DirLights.
			// Does NOT check for duplicates.
			auto RegisterActiveDirLight(DirectionalLight const* dirLight) -> void;

			// Removes the DirLight from the collection of active DirLights.
			// Remove duplicates.
			// Not registered DirLights are ignored.
			auto UnregisterActiveDirLight(DirectionalLight const* dirLight) -> void;

			// Returns the last created active DirectionalLight, or nullptr if none.
			[[nodiscard]] auto DirectionalLight() const -> DirectionalLight const*;

			// Adds the SpotLight to the collection of active SpotLights.
			// Does NOT check for duplicates.
			auto RegisterActiveSpotLight(SpotLight const* spotLight) -> void;

			// Removes the SpotLight from the collection of active SpotLights.
			// Removes duplicates.
			// Not registered SpotLights are ignored.
			auto UnregisterActiveSpotLight(SpotLight const* spotLight) -> void;

			// Returns the registered, currently active SpotLights.
			[[nodiscard]] auto ActiveSpotLights() const noexcept -> std::span<SpotLight const* const>;

			// Adds the PointLight to the collection of active PointLights.
			// Does NOT check for duplicates.
			auto RegisterActivePointLight(PointLight const* pointLight) -> void;

			// Removes the PointLight from the collection of active PointLights.
			// Removes duplicates.
			// Not registered PointLights are ignored.
			auto UnregisterActivePointLight(PointLight const* pointLight) -> void;

			// Returns the registered, currently active PointLights.
			[[nodiscard]] auto ActivePointLights() const noexcept -> std::span<PointLight const* const>;

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
