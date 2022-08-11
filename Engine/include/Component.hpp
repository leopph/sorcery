#pragma once

#include "LeopphApi.hpp"


namespace leopph
{
	class Entity;


	class Component
	{
		friend class Entity;


		public:
			[[nodiscard]] LEOPPHAPI Entity* get_owner() const;


		protected:
			Component() = default;

		public:
			Component(Component const& other) = delete;
			Component& operator=(Component const& other) = delete;

			Component(Component&&) = delete;
			Component& operator=(Component&&) = delete;

			LEOPPHAPI virtual ~Component() = 0;


		private:
			Entity* mOwner{nullptr};
	};
}
