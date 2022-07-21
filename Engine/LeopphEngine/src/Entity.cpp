#include "Entity.hpp"

#include "DataManager.hpp"
#include "InternalContext.hpp"
#include "Logger.hpp"
#include "Types.hpp"

#include <algorithm>
#include <format>
#include <iterator>


namespace leopph
{
	Entity* Entity::find(std::string const& name)
	{
		return internal::GetDataManager()->FindEntity(name);
	}


	std::string const& Entity::get_name() const noexcept
	{
		return mName;
	}


	Transform const& Entity::get_transform() const noexcept
	{
		return mTransform;
	}


	Transform& Entity::get_transform() noexcept
	{
		return mTransform;
	}


	void Entity::attach_component(ComponentPtr<> const& component)
	{
		component->Attach(this);
	}


	void Entity::detach_component(ComponentPtr<> const& component) const
	{
		auto const& logger = internal::Logger::Instance();

		if (!component)
		{
			logger.Warning(std::format("Ignoring attempt to remove nullptr component from Entity [{}].", mName));
			return;
		}

		if (component->Owner() != this)
		{
			logger.Error(std::format("Ignoring attempt to remove component at [{}] from Entity [{}]: the component is not owned by this Entity.", static_cast<void const*>(component.get()), mName));
			return;
		}

		component->Detach();
	}


	void Entity::activate_all_components() const
	{
		// we copy the pointers because the underlying collection will change through activations
		std::vector<ComponentPtr<>> inactiveComponents;
		std::ranges::copy(get_components(false), std::back_inserter(inactiveComponents));

		for (auto const& component : inactiveComponents)
		{
			component->Activate();
		}
	}


	void Entity::deactive_all_components() const
	{
		// we copy the pointers because the underlying collection will change through deactivations
		std::vector<ComponentPtr<>> activeComponents;
		std::ranges::copy(get_components(true), std::back_inserter(activeComponents));

		for (auto const& component : activeComponents)
		{
			component->Deactivate();
		}
	}


	std::span<ComponentPtr<> const> Entity::get_components(bool const active) const
	{
		return internal::GetDataManager()->ComponentsOfEntity(this, active);
	}


	Entity::Entity()
	{
		for (i64 i = 2; internal::GetDataManager()->FindEntity(mName); i++)
		{
			mName = std::format("{} ({})", DEFAULT_NAME, i);
		}

		internal::GetDataManager()->RegisterEntity(this);
	}


	Entity::Entity(std::string_view const name) :
		mName{name}
	{
		for (i64 i = 2; internal::GetDataManager()->FindEntity(mName); i++)
		{
			mName = std::format("{} ({})", name, i);
		}

		internal::GetDataManager()->RegisterEntity(this);
	}


	Entity::Entity(Entity const& other) :
		mName{other.mName}
	{
		for (i64 i = 2; internal::GetDataManager()->FindEntity(mName); i++)
		{
			mName = std::format("{} ({})", other.mName, i);
		}

		mTransform.set_parent(other.mTransform.get_parent());
		mTransform.set_position(other.get_transform().get_position());
		mTransform.set_rotation(other.get_transform().get_rotation());
		mTransform.set_scale(other.get_transform().get_scale());

		internal::GetDataManager()->RegisterEntity(this);

		for (auto const active : {true, false})
		{
			for (auto const& component : other.get_components(active))
			{
				component->Clone()->Attach(this);
			}
		}
	}


	Entity::~Entity()
	{
		auto* const dataManager = internal::GetDataManager();

		for (auto const active : {true, false})
		{
			// Copy the pointers becuase the underlying data structure will change throughout the detaches
			std::vector<ComponentPtr<>> components;
			std::ranges::copy(get_components(active), std::back_inserter(components));

			for (auto const& component : components)
			{
				component->Detach();
			}
		}

		dataManager->UnregisterEntity(this);
	}


	char const* const Entity::DEFAULT_NAME{"entity"};


	std::strong_ordering operator<=>(Entity const& left, Entity const& right)
	{
		return left.get_name() <=> right.get_name();
	}


	std::strong_ordering operator<=>(std::string_view const name, Entity const& entity)
	{
		return name <=> entity.get_name();
	}


	std::strong_ordering operator<=>(Entity const& entity, std::string_view const name)
	{
		return entity.get_name() <=> name;
	}


	bool operator==(Entity const& left, Entity const& right)
	{
		return left.get_name() == right.get_name();
	}


	bool operator==(Entity const& entity, std::string_view const name)
	{
		return entity.get_name() == name;
	}


	bool operator==(std::string_view const name, Entity const& entity)
	{
		return name == entity.get_name();
	}
}
