#pragma once

#include "LeopphApi.hpp"
#include "Scene.hpp"

#include <memory>
#include <string>
#include <vector>


namespace leopph
{
	class SceneManager
	{
		public:
			LEOPPHAPI Scene& create_scene(std::string name = "");

			[[nodiscard]] LEOPPHAPI Scene& get_active_scene() const;

			LEOPPHAPI void set_active_scene(Scene& scene);

			SceneManager();

			SceneManager(SceneManager const& other) = delete;
			SceneManager& operator=(SceneManager const& other) = delete;

			SceneManager(SceneManager&& other) = delete;
			SceneManager& operator=(SceneManager&& other) = delete;

			~SceneManager() = default;

		private:
			std::vector<std::unique_ptr<Scene>> mScenes;
			Scene* mActiveScene;
	};
}
