#pragma once

#include <cstddef>
#include <Leopph.hpp>
#include <Leopph.hpp>
#include <optional>
#include <unordered_map>
#include <vector>


namespace demo
{
	class SceneSwitcher final : public leopph::Behavior
	{
		public:
			class Scene
			{
				public:
					auto Add(leopph::Entity* entity) -> void;
					auto Activate() const -> void;
					auto Deactivate() const -> void;
					auto SetActivationCallback(std::function<void()> callback) -> void;

				private:
					std::function<void()> m_ActivationCallback;
					std::vector<leopph::Entity*> m_Entities;
			};


			auto OnFrameUpdate() -> void override;

			[[nodiscard]]
			auto CreateOrGetScene(leopph::KeyCode key) -> Scene&;

		private:
			std::unordered_map<leopph::KeyCode, Scene> m_Scenes;
	};
}
