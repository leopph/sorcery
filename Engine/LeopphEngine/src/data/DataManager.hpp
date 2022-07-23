#pragma once

#include "Behavior.hpp"
#include "Component.hpp"
#include "DirLight.hpp"
#include "Entity.hpp"
#include "Poelo.hpp"
#include "PointLight.hpp"
#include "SpotLight.hpp"
#include "../util/less/PoeloLess.hpp"

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
			void operator=(DataManager const& other) = delete;

			DataManager(DataManager&& other) = delete;
			void operator=(DataManager&& other) = delete;

			~DataManager();

			// Takes ownership of the Poelo instance.
			void Store(std::unique_ptr<Poelo> poelo);

			// Destroys the passed Poelo instance if found.
			// Returns whether deletion took place.
			bool Destroy(Poelo const* poelo);

			// Non-owningly registers an Entity instance.
			// The function does NOT check if the Entity instance is already registered.
			void RegisterEntity(Entity* entity);

			// Unregisters the passed Entity instance.
			// Removes and destroys the attached Components.
			void UnregisterEntity(Entity const* entity);

			// Returns a pointer to the stored Entity with the passed name, or nullptr.
			[[nodiscard]] Entity* FindEntity(std::string const& name);

			// Adds the Component to the Entity's collection of active/inactive Components depending on the value of active.
			void RegisterComponent(Entity const* entity, ComponentPtr<> component, bool active);

			// Removes the Component from the Entity's collection of active/inactive Components depending on the value of active.
			// Returns the removed pointer.
			ComponentPtr<> UnregisterComponent(Entity const* entity, Component const* component, bool active);

			// Returns the Entity's collection of active/inactive Components depending on the value of active.
			[[nodiscard]]
			std::span<ComponentPtr<> const> ComponentsOfEntity(Entity const* entity, bool active) const;

			// Adds the Behavior to the collection of active Behaviors.
			// Does NOT check for duplicates.
			void RegisterActiveBehavior(Behavior* behavior);

			// Removes the Behavior from the collection of active Behaviors.
			// Removes duplicates.
			// Not registered behaviors are ignored.
			void UnregisterActiveBehavior(Behavior const* behavior);

			// Returns the registered, currently active Behaviors.
			[[nodiscard]] std::span<Behavior* const> ActiveBehaviors() const noexcept;

			// Adds the DirLight to the collection of active DirLights.
			// Does NOT check for duplicates.
			void RegisterActiveDirLight(DirectionalLight const* dirLight);

			// Removes the DirLight from the collection of active DirLights.
			// Remove duplicates.
			// Not registered DirLights are ignored.
			void UnregisterActiveDirLight(DirectionalLight const* dirLight);

			// Returns the last created active DirectionalLight, or nullptr if none.
			[[nodiscard]] DirectionalLight const* DirectionalLight() const;

			// Adds the SpotLight to the collection of active SpotLights.
			// Does NOT check for duplicates.
			void RegisterActiveSpotLight(SpotLight const* spotLight);

			// Removes the SpotLight from the collection of active SpotLights.
			// Removes duplicates.
			// Not registered SpotLights are ignored.
			void UnregisterActiveSpotLight(SpotLight const* spotLight);

			// Returns the registered, currently active SpotLights.
			[[nodiscard]] std::span<SpotLight const* const> ActiveSpotLights() const noexcept;

			// Adds the PointLight to the collection of active PointLights.
			// Does NOT check for duplicates.
			void RegisterActivePointLight(PointLight const* pointLight);

			// Removes the PointLight from the collection of active PointLights.
			// Removes duplicates.
			// Not registered PointLights are ignored.
			void UnregisterActivePointLight(PointLight const* pointLight);

			// Returns the registered, currently active PointLights.
			[[nodiscard]] std::span<PointLight const* const> ActivePointLights() const noexcept;

		private:
			struct EntityEntry
			{
				Entity* Entity;
				std::vector<ComponentPtr<>> ActiveComponents;
				std::vector<ComponentPtr<>> InactiveComponents;
			};


			using EntityOrderFunc = std::ranges::less;
			using BehaviorOrderFunc = std::ranges::less;

			void SortEntities();

		public:
			void SortActiveBehaviors();

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
			decltype(m_EntitiesAndComponents)::iterator GetEntityIterator(std::string const& name);

			// Returns a const iterator to the element or past-the-end.
			[[nodiscard]]
			decltype(m_EntitiesAndComponents)::const_iterator GetEntityIterator(std::string const& name) const;

			// Helper function to get const and non-const iterators depending on context.
			[[nodiscard]] static decltype(auto) GetEntityIteratorCommon(auto* self, std::string const& name);
	};



	[[nodiscard]] decltype(auto) DataManager::GetEntityIteratorCommon(auto* const self, std::string const& name)
	{
		auto it{
			std::ranges::lower_bound(self->m_EntitiesAndComponents, name, [](auto const& elemName, auto const& value)
			                         {
				                         return elemName < value;
			                         }, [](auto const& elem) -> auto const&
			                         {
				                         return elem.Entity->get_name();
			                         })
		};
		if (it != self->m_EntitiesAndComponents.end() && *it->Entity == name)
		{
			return it;
		}
		return decltype(it){self->m_EntitiesAndComponents.end()};
	}
}
