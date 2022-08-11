#include "Behavior.hpp"

#include "../InternalContext.hpp"
#include "../data/DataManager.hpp"


namespace leopph
{
	int Behavior::UpdateIndex() const noexcept
	{
		return m_UpdateIndex;
	}



	void Behavior::UpdateIndex(int const index)
	{
		m_UpdateIndex = index;

		if (InUse())
		{
			internal::GetDataManager()->SortActiveBehaviors();
		}
	}



	void Behavior::set_owner(Entity* entity)
	{
		auto* const dataManager = internal::GetDataManager();

		if (InUse())
		{
			dataManager->UnregisterActiveBehavior(this);
		}

		Component::set_owner(entity);

		if (InUse())
		{
			dataManager->RegisterActiveBehavior(this);
		}
	}



	void Behavior::Active(bool const active)
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



	Behavior& Behavior::operator=(Behavior const& other)
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
