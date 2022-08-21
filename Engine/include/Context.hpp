#pragma once

#include "LeopphApi.hpp"


namespace leopph
{
	class Window;
	class SceneManager;
	class Timer;


	LEOPPHAPI Window& get_main_window();
	LEOPPHAPI SceneManager& get_scene_manager();
	LEOPPHAPI Timer const& get_frame_timer();


	namespace internal
	{
		class Renderer;
		Renderer& get_renderer();

		void set_main_window(Window* window);
		void set_scene_manager(SceneManager* sceneManager);
		void set_frame_timer(Timer const* timer);
		void set_renderer(Renderer* renderer);
	}
}
