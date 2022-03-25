#include "Behavior.hpp"

#include "../data/DataManager.hpp"


namespace leopph
{
	auto Behavior::UpdateIndex() const noexcept -> int
	{
		return m_UpdateIndex;
	}


	auto Behavior::UpdateIndex(int const index) -> void
	{
		m_UpdateIndex = index;

		// If we're actively updated we reregister ourselves to trigger a sort based on the new index.
		if (IsAttached() && IsActive())
		{
			auto& dataManager{internal::DataManager::Instance()};
			dataManager.UnregisterBehavior(this, true);
			dataManager.RegisterBehavior(this, true);
		}
	}


	auto Behavior::Activate() -> void
	{
		if (IsActive())
		{
			return;
		}

		Component::Activate();

		if (IsAttached())
		{
			auto& dataManager{internal::DataManager::Instance()};
			dataManager.UnregisterBehavior(this, false);
			dataManager.RegisterBehavior(this, true);
		}
	}


	auto Behavior::Deactivate() -> void
	{
		if (!IsActive())
		{
			return;
		}

		Component::Deactivate();

		if (IsAttached())
		{
			auto& dataManager{internal::DataManager::Instance()};
			dataManager.UnregisterBehavior(this, true);
			dataManager.RegisterBehavior(this, false);
		}
	}


	auto Behavior::Attach(leopph::Entity* entity) -> void
	{
		if (IsAttached())
		{
			return;
		}

		Component::Attach(entity);

		if (IsActive())
		{
			internal::DataManager::Instance().RegisterBehavior(this, IsActive());
		}
	}


	auto Behavior::Detach() -> void
	{
		if (!IsAttached())
		{
			return;
		}

		Component::Detach();

		if (IsActive())
		{
			internal::DataManager::Instance().UnregisterBehavior(this, IsActive());
		}
	}


	Behavior::~Behavior()
	{
		Detach();
	}
}
