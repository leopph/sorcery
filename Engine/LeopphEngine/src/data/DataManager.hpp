#pragma once

#include "Behavior.hpp"
#include "Poelo.hpp"
#include "../util/less/PoeloLess.hpp"

#include <memory>
#include <set>
#include <span>
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



			// Adds the Behavior to the collection of active Behaviors.
			// Does NOT check for duplicates.
			void RegisterActiveBehavior(Behavior* behavior);

			// Removes the Behavior from the collection of active Behaviors.
			// Removes duplicates.
			// Not registered behaviors are ignored.
			void UnregisterActiveBehavior(Behavior const* behavior);

			// Returns the registered, currently active Behaviors.
			[[nodiscard]] std::span<Behavior* const> ActiveBehaviors() const noexcept;

		private:
			using BehaviorOrderFunc = std::ranges::less;

		public:
			void SortActiveBehaviors();

		private:
			// All engine-owned objects.
			std::set<std::unique_ptr<Poelo>, PoeloLess> m_Poelos;

			// Non-owning pointers to active Behaviors.
			std::vector<Behavior*> m_ActiveBehaviors;
	};
}
