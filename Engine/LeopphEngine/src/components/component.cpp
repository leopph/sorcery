#include "Component.hpp"

#include "DataManager.hpp"
#include "InternalContext.hpp"
#include "Logger.hpp"


namespace leopph
{
	Entity* Component::Owner() const noexcept
	{
		return m_Owner;
	}


	void Component::Owner(Entity* entity)
	{
		auto* const dataManager = internal::GetDataManager();

		if (!m_Owner && entity)
		{
			try
			{
				dataManager->RegisterComponent(entity, shared_from_this(), Active());
				auto const oldOwner = m_Owner;
				m_Owner = entity;
				OnOwnerChange(oldOwner);
				OnAttach();
			}
			catch (std::bad_weak_ptr const&)
			{
				auto const errMsg{"Failed to attach Component because it is not a shared object."};
				internal::Logger::Instance().Critical(errMsg);
				throw std::runtime_error{errMsg};
			}
		}
		else if (m_Owner && !entity)
		{
			auto const self = dataManager->UnregisterComponent(m_Owner, this, Active()); // self might be the last pointer to this
			auto const oldOwner = m_Owner;
			m_Owner = entity;
			OnOwnerChange(oldOwner);
			OnDetach();
		}
		else if (m_Owner && entity)
		{
			auto self = dataManager->UnregisterComponent(m_Owner, this, Active());
			dataManager->RegisterComponent(entity, std::move(self), Active());
			auto const oldOwner = m_Owner;
			m_Owner = entity;
			OnOwnerChange(oldOwner);
		}
	}


	void Component::Attach(Entity* entity)
	{
		Owner(entity);
	}


	void Component::Detach()
	{
		Owner(nullptr);
	}


	bool Component::Attached() const
	{
		return Owner() != nullptr;
	}


	bool Component::Active() const noexcept
	{
		return m_Active;
	}


	void Component::Active(bool const active)
	{
		if (m_Active != active)
		{
			if (Attached())
			{
				auto* const dataManager = internal::GetDataManager();
				auto self = dataManager->UnregisterComponent(m_Owner, this, m_Active);
				dataManager->RegisterComponent(m_Owner, std::move(self), active);
			}
			m_Active = active;
			m_Active ? OnActivate() : OnDeactivate();
		}
	}


	void Component::Activate()
	{
		Active(true);
	}


	void Component::Deactivate()
	{
		Active(false);
	}


	bool Component::InUse() const noexcept
	{
		return Owner() && Active();
	}


	Component::Component(Component const& other) :
		std::enable_shared_from_this<Component>{other},
		m_Active{other.m_Active}
	{}


	Component& Component::operator=(Component const& other)
	{
		if (this == &other)
		{
			return *this;
		}

		Active(other.Active());
		Owner(other.Owner());

		return *this;
	}


	ComponentPtr<> Component::Clone() const
	{
		throw std::logic_error{"Unimplemented Clone member function called."};
	}


	Component::~Component() = default;
}
