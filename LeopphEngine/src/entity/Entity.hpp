#pragma once

#include "../api/LeopphApi.hpp"
#include "../components/Component.hpp"
#include "../components/Transform.hpp"
#include "../data/Poelo.hpp"

#include <concepts>
#include <memory>
#include <span>
#include <string>
#include <utility>


namespace leopph
{
	// Entities form the basis of the object hierarchy.
	// They have spatial properties through an unremovable Transform component.
	// They can also be decorated with custom Components.
	class Entity final : internal::Poelo
	{
		public:
			// Returns a pointer to the Entity with the passed name, or nullptr if not found.
			LEOPPHAPI static
			auto FindEntity(const std::string& name) -> Entity*;

			// The Entity's name is a unique identifier.
			constexpr
			auto Name() const noexcept -> auto&;

			// The Entity's Transform describes its spatial properties.
			[[nodiscard]] constexpr
			auto Transform() const noexcept;

			// Returns the first active Component of type T attached to the Entity the engine finds, or nullptr.
			// There are no guarantees of the order of Components attached to the Entity.
			template<std::derived_from<Component> T>
			auto GetComponent() const -> T*;

			// Attach an already existing Component to the Entity.
			// Equivalent to calling component->Attach(this).
			LEOPPHAPI
			auto AttachComponent(Component* component) -> void;

			// Attach a newly constructed Component of type T to the Entity.
			// Returns a pointer the Component.
			// If the Component could not be attached, it will still be kept alive and returned unattached.
			template<std::derived_from<Component> T, class... Args>
			auto CreateAndAttachComponent(Args&&... args);

			// Detach the given Component from the Entity.
			LEOPPHAPI
			auto DetachComponent(Component* component) const -> void;

			// Activates all inactive Components attached to the Entity.
			// Equivalent to calling Activate() on all attached inactive Components.
			LEOPPHAPI
			auto ActivateAllComponents() const -> void;

			// Deactivates all active Components attached to the Entity.
			// Equivalent to calling Deactivate() on all attached active Components.
			LEOPPHAPI
			auto DeactiveAllComponents() const -> void;

			LEOPPHAPI explicit
			Entity(std::string name = GenerateUnusedName());

			Entity(const Entity&) = delete;
			auto operator=(const Entity&) -> void = delete;

			Entity(Entity&&) = delete;
			auto operator=(Entity&&) -> void = delete;

			// Detaches all Components, then unregisters.
			~Entity() override;

		private:
			// Generate a name that is not used by any registered Entity at the time of calling.
			// The function uses the passed prefix and appends arbitrary characters to its end.
			LEOPPHAPI static
			auto GenerateUnusedName(const std::string& namePrefix = "Entity#") -> std::string;

			static
			auto NameIsUnused(const std::string& name) -> bool;

			// The active Components attached to the Entity.
			[[nodiscard]] LEOPPHAPI
			auto Components() const -> std::span<Component* const>;

			std::string m_Name;
			// This has to be attached after registering the Entity.
			leopph::Transform* m_Transform{new leopph::Transform{}};
	};


	constexpr auto Entity::Name() const noexcept -> auto&
	{
		return m_Name;
	}


	constexpr auto Entity::Transform() const noexcept
	{
		return m_Transform;
	}


	template<std::derived_from<Component> T, class... Args>
	auto Entity::CreateAndAttachComponent(Args&&... args)
	{
		const auto component{new T{std::forward<Args>(args)...}};
		AttachComponent(component);
		return component;
	}


	template<std::derived_from<Component> T>
	auto Entity::GetComponent() const -> T*
	{
		for (const auto& component : Components())
		{
			if (const auto ret = dynamic_cast<T*>(component);
				ret != nullptr)
			{
				return ret;
			}
		}
		return nullptr;
	}


	// Transforms cannot be explicitly created.
	template<>
	auto Entity::CreateAndAttachComponent<Transform>() = delete;
}
