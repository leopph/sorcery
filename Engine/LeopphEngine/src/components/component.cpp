#include "Component.hpp"

#include "DataManager.hpp"
#include "InternalContext.hpp"
#include "Logger.hpp"


namespace leopph
{
	auto Component::Owner() const noexcept -> Entity*
	{
		return m_Owner;
	}


	auto Component::Owner(Entity* entity) -> void
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


	auto Component::Attach(Entity* entity) -> void
	{
		Owner(entity);
	}


	auto Component::Detach() -> void
	{
		Owner(nullptr);
	}


	auto Component::Attached() const -> bool
	{
		return Owner() != nullptr;
	}


	auto Component::Active() const noexcept -> bool
	{
		return m_Active;
	}


	auto Component::Active(bool const active) -> void
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


	auto Component::Activate() -> void
	{
		Active(true);
	}


	auto Component::Deactivate() -> void
	{
		Active(false);
	}


	auto Component::InUse() const noexcept -> bool
	{
		return Owner() && Active();
	}


	Component::Component(Component const& other) :
		std::enable_shared_from_this<Component>{other},
		m_Active{other.m_Active}
	{}


	auto Component::operator=(Component const& other) -> Component&
	{
		if (this == &other)
		{
			return *this;
		}

		Active(other.Active());
		Owner(other.Owner());

		return *this;
	}


	auto Component::Clone() const -> ComponentPtr<>
	{
		throw std::logic_error{"Unimplemented Clone member function called."};
	}


	Component::~Component() = default;
}
