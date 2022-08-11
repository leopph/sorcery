#pragma once

#include "LeopphApi.hpp"


namespace leopph
{
	class Window;
	class SceneManager;


	LEOPPHAPI Window* get_window();
	LEOPPHAPI SceneManager* get_scene_manager();


	namespace internal
	{
		class Renderer;
		Renderer* get_renderer();

		void set_window(Window* window);
		void set_scene_manager(SceneManager* sceneManager);
		void set_renderer(Renderer* renderer);
	}
}
