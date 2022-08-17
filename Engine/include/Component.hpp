#pragma once

#include "LeopphApi.hpp"

#include <vector>


namespace leopph
{
	class Entity;


	class Component
	{
		friend class Entity;


		public:
			inline virtual void on_init();

			[[nodiscard]] LEOPPHAPI Entity* get_owner() const;

			static void init_all();

		protected:
			LEOPPHAPI Component();

		public:
			Component(Component const& other);
			Component(Component&&) noexcept;

			Component& operator=(Component const& other) = default;
			Component& operator=(Component&&) noexcept = default;

			LEOPPHAPI virtual ~Component() = 0;


		private:
			Entity* mOwner{nullptr};

			static std::vector<Component*> sUninitialized;
	};



	inline void Component::on_init()
	{ }
}
