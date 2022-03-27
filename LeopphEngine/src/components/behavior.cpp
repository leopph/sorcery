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

		if (InUse())
		{
			internal::DataManager::Instance().SortActiveBehaviors();
		}
	}


	auto Behavior::Owner(Entity* entity) -> void
	{
		auto& dataManager = internal::DataManager::Instance();

		if (InUse())
		{
			dataManager.UnregisterActiveBehavior(this);
		}

		Component::Owner(entity);

		if (InUse())
		{
			dataManager.RegisterActiveBehavior(this);
		}
	}


	auto Behavior::Active(bool const active) -> void
	{
		auto& dataManager = internal::DataManager::Instance();

		if (InUse())
		{
			dataManager.UnregisterActiveBehavior(this);
		}

		Component::Active(active);

		if (InUse())
		{
			dataManager.RegisterActiveBehavior(this);
		}
	}


	Behavior::~Behavior()
	{
		internal::DataManager::Instance().UnregisterActiveBehavior(this);
	}


	auto Behavior::operator=(Behavior const& other) -> Behavior&
	{
		if (this == &other)
		{
			return *this;
		}

		auto& dataManager = internal::DataManager::Instance();

		if (InUse())
		{
			dataManager.UnregisterActiveBehavior(this);
		}

		Component::operator=(other);
		UpdateIndex(other.m_UpdateIndex);

		if (InUse())
		{
			dataManager.RegisterActiveBehavior(this);
		}

		return *this;
	}
}
