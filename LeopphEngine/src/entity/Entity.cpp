#include "Entity.hpp"

#include "../data/DataManager.hpp"
#include "../util/Logger.hpp"

#include <algorithm>
#include <array>
#include <cstddef>


namespace leopph
{
	auto Entity::Find(std::string const& name) -> Entity*
	{
		return internal::DataManager::Instance().FindEntity(name);
	}


	auto Entity::Name() const noexcept -> std::string const&
	{
		return m_Name;
	}


	auto Entity::Transform() const noexcept -> ComponentPtr<leopph::Transform> const&
	{
		return m_Transform;
	}


	auto Entity::AttachComponent(ComponentPtr<> const& component) -> void
	{
		component->Attach(this);
	}


	auto Entity::DetachComponent(ComponentPtr<> const& component) const -> void
	{
		auto const& logger{internal::Logger::Instance()};

		if (!component)
		{
			logger.Warning("Ignoring attempt to remove nullptr component from Entity [" + m_Name + "].");
			return;
		}

		if (component->Owner() != this)
		{
			internal::Logger::Instance().Error("Ignoring attempt to remove component at [" + std::to_string(reinterpret_cast<std::size_t>(component.get())) + "] from Entity [" + m_Name + "], because the component is not owned by the Entity.");
			return;
		}

		component->Detach();
	}


	auto Entity::ActivateAllComponents() const -> void
	{
		std::span<ComponentPtr<> const> components;

		// Elements are removed from the container on activation so we have to recall for each component.
		while (!(components = internal::DataManager::Instance().ComponentsOfEntity(this, false)).empty())
		{
			components[0]->Activate();
		}
	}


	auto Entity::DeactiveAllComponents() const -> void
	{
		std::span<ComponentPtr<> const> components;

		// Elements are removed from the container on deactivation so we have to recall for each component.
		while (!(components = internal::DataManager::Instance().ComponentsOfEntity(this, true)).empty())
		{
			components[0]->Deactivate();
		}
	}


	Entity::Entity(std::string name) :
		m_Name{
			[&name]
			{
				if (!NameIsUnused(name))
				{
					name = GenerateUnusedName(name);
					internal::Logger::Instance().Warning("Name collision detected. Entity is being renamed to " + name + ".");
				}

				return name;
			}()
		},
		m_Transform{CreateComponent<leopph::Transform>()}
	{
		internal::DataManager::Instance().RegisterEntity(this);
		m_Transform->Attach(this);
	}


	Entity::Entity(Entity const& other) :
		m_Name{GenerateUnusedName(other.m_Name)}
	{
		internal::DataManager::Instance().RegisterEntity(this);

		for (auto const& component : other.Components())
		{
			component->Clone()->Attach(this);
		}

		m_Transform = GetComponent<leopph::Transform>();
	}


	auto Entity::operator=(Entity const& other) -> Entity&
	{
		auto& dataManager = internal::DataManager::Instance();

		m_Transform->Component::Owner(nullptr);

		std::ranges::for_each(std::array{true, false}, [this, &dataManager](auto const active)
		{
			auto components = dataManager.ComponentsOfEntity(this, active);
			// Detach erases itself from the component collection, so we iterate backwards to not cause element relocation
			std::for_each(components.rbegin(), components.rend(), [](auto const component)
			{
				component->Detach();
			});
		});

		dataManager.UnregisterEntity(this);

		m_Name = GenerateUnusedName(other.m_Name);

		dataManager.RegisterEntity(this);

		std::ranges::for_each(other.Components(), [this](auto const& component)
		{
			component->Clone()->Attach(this);
		});

		m_Transform = GetComponent<leopph::Transform>();

		return *this;
	}


	Entity::~Entity()
	{
		auto& dataManager{internal::DataManager::Instance()};

		m_Transform->Component::Owner(nullptr); // Transform is a special case because it cannot be detached.

		std::ranges::for_each(std::array{true, false}, [this, &dataManager](auto const active)
		{
			auto components = dataManager.ComponentsOfEntity(this, active);
			// Detach erases itself from the component collection, so we iterate backwards to not cause element relocation
			std::for_each(components.rbegin(), components.rend(), [](auto const component)
			{
				component->Detach();
			});
		});

		dataManager.UnregisterEntity(this);
	}


	auto Entity::operator<=>(Entity const& other) const noexcept -> std::strong_ordering
	{
		return m_Name <=> other.m_Name;
	}


	auto Entity::operator==(Entity const& other) const noexcept -> bool
	{
		return m_Name == other.m_Name;
	}


	auto Entity::Components() const -> std::span<ComponentPtr<> const>
	{
		return internal::DataManager::Instance().ComponentsOfEntity(this, true);
	}


	auto Entity::GenerateUnusedName(std::string const& original) -> std::string
	{
		auto newName = original + "0";

		for (std::size_t i = 1; !NameIsUnused(newName); i++)
		{
			newName.replace(newName.size() - 1, 1, std::to_string(i));
		}

		return newName;
	}


	auto Entity::NameIsUnused(std::string const& name) -> bool
	{
		return !internal::DataManager::Instance().FindEntity(name);
	}
}
