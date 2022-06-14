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
			auto Find(std::string const& name) -> Entity*;

			// The Entity's name is a unique identifier.
			[[nodiscard]] LEOPPHAPI
			auto Name() const noexcept -> std::string const&;

			// The Entity's Transform describes its spatial properties.
			[[nodiscard]] LEOPPHAPI
			auto Transform() const noexcept -> ComponentPtr<Transform> const&;

			// Returns the first active Component of type T attached to the Entity the engine finds, or nullptr.
			// There are no guarantees of the order of Components attached to the Entity.
			template<std::derived_from<Component> T>
			[[nodiscard]]
			auto GetComponent() const -> ComponentPtr<T>;

			// Attach an already existing Component to the Entity.
			// Equivalent to calling component->Attach(this).
			LEOPPHAPI
			auto AttachComponent(ComponentPtr<> const& component) -> void;

			// Attach a newly constructed Component of type T to the Entity.
			// Returns a pointer the Component.
			// If the Component could not be attached, it will still be kept alive and returned unattached.
			template<std::derived_from<Component> T, class... Args>
			auto CreateAndAttachComponent(Args&&... args);

			// Detach the given Component from the Entity.
			LEOPPHAPI
			auto DetachComponent(ComponentPtr<> const& component) const -> void;

			// Activates all inactive Components attached to the Entity.
			// Equivalent to calling Activate() on all attached inactive Components.
			LEOPPHAPI
			auto ActivateAllComponents() const -> void;

			// Deactivates all active Components attached to the Entity.
			// Equivalent to calling Deactivate() on all attached active Components.
			LEOPPHAPI
			auto DeactiveAllComponents() const -> void;

			LEOPPHAPI explicit Entity(std::string name = "Entity");

			// Copies all of other's Components and attaches them.
			// Name generation is the as if trying to create a new component with the same name as other's.
			LEOPPHAPI Entity(Entity const& other);

			// Detaches all existing components.
			// Copies all Components from other and attaches them.
			// Name generation is the as if trying to create a new component with the same name as other's.
			LEOPPHAPI
			auto operator=(Entity const& other) -> Entity&;

			Entity(Entity&& other) = delete;
			auto operator=(Entity&& other) -> void = delete;

			// Detaches all Components, then unregisters.
			~Entity() override;

			// Provides ordering based on name.
			[[nodiscard]] LEOPPHAPI
			auto operator<=>(Entity const& other) const noexcept -> std::strong_ordering;

			// Equality based on names.
			[[nodiscard]] LEOPPHAPI
			auto operator==(Entity const& other) const noexcept -> bool;

		private:
			[[nodiscard]] static
			auto NameIsUnused(std::string const& name) -> bool;

			[[nodiscard]] static
			auto GenerateUnusedName(std::string const& original) -> std::string;

			// The active Components attached to the Entity.
			[[nodiscard]] LEOPPHAPI
			auto Components() const -> std::span<ComponentPtr<> const>;

			std::string m_Name;
			// This has to be created in constructor and attached after registering the Entity.
			ComponentPtr<leopph::Transform> m_Transform;
	};


	template<std::derived_from<Component> T, class... Args>
	auto Entity::CreateAndAttachComponent(Args&&... args)
	{
		auto component{CreateComponent<T>(std::forward<Args>(args)...)};
		AttachComponent(component);
		return component;
	}


	template<std::derived_from<Component> T>
	auto Entity::GetComponent() const -> ComponentPtr<T>
	{
		for (auto const& component : Components())
		{
			if (auto ret = std::dynamic_pointer_cast<T>(component))
			{
				return ret;
			}
		}
		return nullptr;
	}


	// Transforms cannot be explicitly created.
	template<>
	auto Entity::CreateAndAttachComponent<Transform>() = delete;

	// Provides ordering with strings.
	[[nodiscard]] inline
	auto operator<=>(Entity const& entity, std::string_view const name)
	{
		return entity.Name() <=> name;
	}


	// Provides ordering with strings.
	[[nodiscard]] inline
	auto operator<=>(std::string_view const name, Entity const& entity)
	{
		return name <=> entity.Name();
	}


	// Provides equality with strings.
	[[nodiscard]] inline
	auto operator==(Entity const& entity, std::string_view const name) -> bool
	{
		return entity.Name() == name;
	}


	// Provides equality with strings.
	[[nodiscard]] inline
	auto operator==(std::string_view const name, Entity const& entity) -> bool
	{
		return name == entity.Name();
	}
}
