#pragma once

#include "LeopphApi.hpp"

#include <cstddef>
#include <string_view>


namespace leopph
{
	class Scene
	{
		public:
			[[nodiscard]] LEOPPHAPI static Scene* CreateScene(std::size_t id);
			[[nodiscard]] LEOPPHAPI static Scene* CreateScene(std::string_view name);
			[[nodiscard]] LEOPPHAPI static std::size_t NameToId(std::string_view name);

			[[nodiscard]] constexpr auto Id() const noexcept;

		private:
			explicit Scene(std::size_t id);

			std::size_t m_Id;

			friend class SceneManager;
	};


	constexpr auto Scene::Id() const noexcept
	{
		return m_Id;
	}
}
