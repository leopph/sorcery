#pragma once

#include "../api/LeopphApi.hpp"
#include "../components/Component.hpp"
#include "../components/Transform.hpp"

#include <concepts>
#include <memory>
#include <string>
#include <utility>
#include <vector>


namespace leopph
{
	/* Entities are kind of object skeletons and form the basis of the object hierarchy.
	 * These are the objects Components must be attached to in order to have an effect at runtime.
	 * Entities are provided spatial properties through a Transform component. */
	class Entity final
	{
		public:
			/* Creates a new Entity instance and returns a pointer to it.
			 * If the passed name is already in use, LeopphEngine generates an undefined sequence of characters and appends it to the desired name. */
			LEOPPHAPI static auto CreateEntity(std::string name = GenerateUnusedName()) -> Entity*;

			/* Destroys the passed in Entity.
			 * This also destroys attached Components.
			 * Leftover pointers to either of them will be dangling. */
			LEOPPHAPI static auto DestroyEntity(const Entity* entity) -> void;

			/* Returns a pointer to the Entity that's name is equal to the given string.
			 * Returns NULL if no such Entity exists. */
			LEOPPHAPI static auto FindEntity(const std::string& name) -> Entity*;

			/* Attach a newly constructed Component of type T to the Entity.
			 * Returns a non-owning pointer the Component. */
			template<std::derived_from<Component> T, class... Args>
			auto CreateComponent(Args&&... args) -> T*
			{
				auto component{std::make_unique<T>(this, std::forward<Args>(args)...)};
				auto ret{component.get()};
				RegisterComponent(std::move(component));
				return ret;
			}

			/* Remove the given Component from the Entity.
			 * The component is detached and destroyed. */
			LEOPPHAPI auto RemoveComponent(const Component* component) const -> void;

			/* Look for a Component of type T that is attached to the Entity.
			 * If there is none, NULL is returned.
			 * Otherwise, a pointer to the first matching Component is returned.
			 * There are no guarantees of the order of Components attached to the Entity. */
			template<std::derived_from<Component> T>
			auto GetComponent() const -> T*
			{
				for (const auto& component : Components())
				{
					if (const auto ret = dynamic_cast<T*>(component.get());
						ret != nullptr)
					{
						return ret;
					}
				}
				return nullptr;
			}

			/* The Entity's name is a unique identifier. */
			LEOPPHAPI auto Name() const -> const std::string&;

			/* The Entity's Transform describes its spatial properties. */
			LEOPPHAPI auto Transform() const -> Transform*;

			Entity(const Entity&) = delete;
			auto operator=(const Entity&) -> void = delete;

			Entity(Entity&&) = delete;
			auto operator=(Entity&&) -> void = delete;

			LEOPPHAPI ~Entity() noexcept = default;

		private:
			/* Generate a name that is not used by any registered Entity at the time of calling.
			 * The function uses the passed prefix and appends arbitrary characters to its end. */
			LEOPPHAPI static auto GenerateUnusedName(const std::string& namePrefix = "Entity#") -> std::string;
			static auto NameIsUnused(const std::string& name) -> bool;

			explicit Entity(std::string name);

			// Registers the passed Component in DataManager.
			LEOPPHAPI auto RegisterComponent(std::unique_ptr<Component>&& component) const -> void;

			// Returns a collection of Components attached to the Entity.
			[[nodiscard]]
			LEOPPHAPI auto Components() const -> const std::vector<std::unique_ptr<Component>>&;

			std::string m_Name;
			mutable leopph::Transform* m_TransformCache;
	};


	// Transforms cannot be explicitly created.
	template<>
	auto Entity::CreateComponent() -> leopph::Transform* = delete;
}
