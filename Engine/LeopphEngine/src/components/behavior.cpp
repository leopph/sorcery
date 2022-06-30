#include "Behavior.hpp"

#include "DataManager.hpp"
#include "InternalContext.hpp"


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
			internal::GetDataManager()->SortActiveBehaviors();
		}
	}


	auto Behavior::Owner(Entity* entity) -> void
	{
		auto* const dataManager = internal::GetDataManager();

		if (InUse())
		{
			dataManager->UnregisterActiveBehavior(this);
		}

		Component::Owner(entity);

		if (InUse())
		{
			dataManager->RegisterActiveBehavior(this);
		}
	}


	auto Behavior::Active(bool const active) -> void
	{
		auto* const dataManager = internal::GetDataManager();

		if (InUse())
		{
			dataManager->UnregisterActiveBehavior(this);
		}

		Component::Active(active);

		if (InUse())
		{
			dataManager->RegisterActiveBehavior(this);
		}
	}


	Behavior::~Behavior()
	{
		internal::GetDataManager()->UnregisterActiveBehavior(this);
	}


	auto Behavior::operator=(Behavior const& other) -> Behavior&
	{
		if (this == &other)
		{
			return *this;
		}

		auto* const dataManager = internal::GetDataManager();

		if (InUse())
		{
			dataManager->UnregisterActiveBehavior(this);
		}

		Component::operator=(other);
		UpdateIndex(other.m_UpdateIndex);

		if (InUse())
		{
			dataManager->RegisterActiveBehavior(this);
		}

		return *this;
	}
}
