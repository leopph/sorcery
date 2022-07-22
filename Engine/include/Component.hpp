#pragma once

#include "LeopphApi.hpp"

#include <concepts>
#include <memory>


namespace leopph
{
	class Entity;
	class Component;

	// Reference counted pointer to a Component with ownership.
	template<std::derived_from<Component> T = Component>
	using ComponentPtr = std::shared_ptr<T>;

	template<std::derived_from<Component> T, typename... Args>
	constexpr ComponentPtr<T> CreateComponent(Args&&... args);

	// Special objects used that add custom properties to Entities.
	// Components are attached to Entities to achieve this decoration.
	// The Entity they are attached to is said to be their owner.
	// Components can also be active or inactive.
	// Only attached and active Components take part in internal calculations.
	class Component : public std::enable_shared_from_this<Component>
	{
		public:
			// Get the owner of the Component or nullptr if none.
			[[nodiscard]] LEOPPHAPI Entity* Owner() const noexcept;

			// Set the owner of the Component.
			// Passing nullptr will leave the Component unattached.
			// If the Component already has an owner, it will detach itself first.
			LEOPPHAPI virtual void Owner(Entity* entity);

			// Same as calling Owner with a non-null pointer.
			// Hence, nullptr must not be passed.
			LEOPPHAPI void Attach(Entity* entity);

			// Same as calling Owner with nullptr.
			LEOPPHAPI void Detach();

			// Returns whether the Component has an owner.
			// Same as Owner() != nullptr.
			[[nodiscard]] LEOPPHAPI bool Attached() const;

			// Get whether the Component is active.
			[[nodiscard]] LEOPPHAPI bool Active() const noexcept;

			// Set whether the Component is active.
			LEOPPHAPI virtual void Active(bool active);

			// Same as calling Active(true).
			LEOPPHAPI void Activate();

			// Same as calling Active(false);
			LEOPPHAPI void Deactivate();

			// Returns whether the Component takes part in internal calculations.
			// Same as Owner() && Active().
			[[nodiscard]] LEOPPHAPI bool InUse() const noexcept;

			// Gets called after the Component changed owner.
			LEOPPHAPI inline virtual void OnOwnerChange(Entity* oldOwner);

			// Gets called after the Component was attached.
			LEOPPHAPI inline virtual void OnAttach();

			// Gets called after the Component was detached.
			LEOPPHAPI inline virtual void OnDetach();

			// Gets called when the Component was activated.
			LEOPPHAPI inline virtual void OnActivate();

			// Gets called when the Component was deactivated.
			LEOPPHAPI inline virtual void OnDeactivate();

			// Returns a new Component that is a dynamic copy of this Component.
			// This can be used to get an unsliced copy when the exact type is unknown.
			// Throws std::logic_error unless correctly overloaded.
			// A fair implementation is calling CreateComponent<MyComponentType>(*this), assuming a correct copy constructor is defined.
			// Built-in Components use this exact implementation.
			[[nodiscard]] LEOPPHAPI virtual ComponentPtr<> Clone() const;

			Component(Component&&) = delete;
			Component& operator=(Component&&) = delete;

			LEOPPHAPI virtual ~Component() = 0;

		protected:
			Component() = default;

			// The new Component will have the same active state but will be unattached.
			LEOPPHAPI Component(Component const& other);

			// Attaches itself to the the Component's Entity and takes its active state.
			LEOPPHAPI Component& operator=(Component const& other);

		private:
			Entity* m_Owner{nullptr};
			bool m_Active{true};
	};


	template<std::derived_from<Component> T, typename... Args>
	constexpr ComponentPtr<T> CreateComponent(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}


	inline void Component::OnOwnerChange(Entity*)
	{ }


	inline void Component::OnAttach()
	{ }


	inline void Component::OnDetach()
	{ }


	inline void Component::OnActivate()
	{ }


	inline void Component::OnDeactivate()
	{ }
}
