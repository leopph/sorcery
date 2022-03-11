#pragma once

#include <cstddef>
#include <Leopph.hpp>
#include <optional>
#include <unordered_map>
#include <vector>


namespace demo
{
	class SceneSwitcher
	{
		public:
			// Registers the given entities as a scene.
			// Returns the id given to the scene.
			[[nodiscard]]
			auto RegisterScene(std::vector<leopph::Entity*> entities) -> std::size_t;

			// Deactivates all components in the active scene if any,
			// then activates all components in the scene with the passed id.
			auto ActivateScene(std::size_t id) -> void;

		private:
			[[nodiscard]] static
			auto GenerateId() noexcept -> std::size_t;

			std::optional<std::size_t> m_Active;
			std::unordered_map<std::size_t, std::vector<leopph::Entity*>> m_Scenes;
	};
}
