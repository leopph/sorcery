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
					void Add(leopph::Entity* entity);
					void Activate() const;
					void Deactivate() const;
					void SetActivationCallback(std::function<void()> callback);

				private:
					std::function<void()> m_ActivationCallback;
					std::vector<leopph::Entity*> m_Entities;
			};


			void on_frame_update() override;

			[[nodiscard]]
			Scene& CreateOrGetScene(leopph::KeyCode key);

		private:
			std::unordered_map<leopph::KeyCode, Scene> m_Scenes;
	};
}
