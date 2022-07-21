#pragma once

#include "Component.hpp"
#include "LeopphApi.hpp"
#include "Poelo.hpp"
#include "Transform.hpp"

#include <compare>
#include <concepts>
#include <memory>
#include <span>
#include <string>
#include <utility>


namespace leopph
{
	class Entity final : internal::Poelo
	{
		public:
			LEOPPHAPI static Entity* find(std::string const& name);

			[[nodiscard]] LEOPPHAPI std::string const& get_name() const noexcept;

			[[nodiscard]] LEOPPHAPI Transform& get_transform() noexcept;
			[[nodiscard]] LEOPPHAPI Transform const& get_transform() const noexcept;


			template<std::derived_from<Component> T>
			[[nodiscard]] ComponentPtr<T> get_component() const;


			LEOPPHAPI void attach_component(ComponentPtr<> const& component);
			LEOPPHAPI void detach_component(ComponentPtr<> const& component) const;

			template<std::derived_from<Component> T, class... Args>
			auto create_and_attach_component(Args&&... args);


			LEOPPHAPI void activate_all_components() const;
			LEOPPHAPI void deactive_all_components() const;


		private:
			[[nodiscard]] LEOPPHAPI std::span<ComponentPtr<> const> get_components(bool active) const;


		public:
			LEOPPHAPI Entity();
			LEOPPHAPI explicit Entity(std::string_view name);

			LEOPPHAPI Entity(Entity const& other);
			Entity& operator=(Entity const& other) = delete;

			Entity(Entity&& other) = delete;
			void operator=(Entity&& other) = delete;

			~Entity() override;

		private:
			static char const* const DEFAULT_NAME;

			std::string mName{DEFAULT_NAME};
			Transform mTransform{this};
	};


	[[nodiscard]] LEOPPHAPI std::strong_ordering operator<=>(Entity const& left, Entity const& right);
	[[nodiscard]] LEOPPHAPI std::strong_ordering operator<=>(std::string_view name, Entity const& entity);
	[[nodiscard]] LEOPPHAPI std::strong_ordering operator<=>(Entity const& entity, std::string_view name);


	[[nodiscard]] LEOPPHAPI bool operator==(Entity const& left, Entity const& right);
	[[nodiscard]] LEOPPHAPI bool operator==(Entity const& entity, std::string_view name);
	[[nodiscard]] LEOPPHAPI bool operator==(std::string_view name, Entity const& entity);



	template<std::derived_from<Component> T, class... Args>
	auto Entity::create_and_attach_component(Args&&... args)
	{
		auto component = CreateComponent<T>(std::forward<Args>(args)...);
		attach_component(component);
		return component;
	}


	template<std::derived_from<Component> T>
	ComponentPtr<T> Entity::get_component() const
	{
		for (auto const active : {true, false})
		{
			for (auto const& component : get_components(active))
			{
				if (auto ret = std::dynamic_pointer_cast<T>(component))
				{
					return ret;
				}
			}
		}
		return nullptr;
	}
}
