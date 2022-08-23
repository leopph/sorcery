#include "Context.hpp"


namespace leopph
{
	namespace
	{
		Window* gMainWindow;
		SceneManager* gSceneManager;
		Timer const* gFrameTimer;
	}


	namespace internal
	{
		namespace
		{
			Renderer* gRenderer;
		}
	}



	Window& get_main_window()
	{
		return *gMainWindow;
	}



	SceneManager& get_scene_manager()
	{
		return *gSceneManager;
	}



	Timer const& get_frame_timer()
	{
		return *gFrameTimer;
	}



	namespace internal
	{
		Renderer& get_renderer()
		{
			return *gRenderer;
		}



		void set_main_window(Window* const window)
		{
			gMainWindow = window;
		}



		void set_scene_manager(SceneManager* const sceneManager)
		{
			gSceneManager = sceneManager;
		}



		void set_frame_timer(Timer const* const timer)
		{
			gFrameTimer = timer;
		}



		void set_renderer(Renderer* const renderer)
		{
			gRenderer = renderer;
		}
	}
}
