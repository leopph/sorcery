#pragma once

#include "LeopphApi.hpp"

#include <cstddef>
#include <string_view>


namespace leopph
{
	class Scene
	{
		public:
			[[nodiscard]] LEOPPHAPI static auto CreateScene(std::size_t id) -> Scene*;
			[[nodiscard]] LEOPPHAPI static auto CreateScene(std::string_view name) -> Scene*;
			[[nodiscard]] LEOPPHAPI static auto NameToId(std::string_view name) -> std::size_t;

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
