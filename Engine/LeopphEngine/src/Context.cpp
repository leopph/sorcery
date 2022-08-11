#include "Context.hpp"


namespace leopph
{
	namespace
	{
		Window* gWindow;
		SceneManager* gSceneManager;
	}


	namespace internal
	{
		namespace
		{
			Renderer* gRenderer;
		}
	}



	Window* get_window()
	{
		return gWindow;
	}



	SceneManager* get_scene_manager()
	{
		return gSceneManager;
	}



	namespace internal
	{
		Renderer* get_renderer()
		{
			return gRenderer;
		}



		void set_window(Window* const window)
		{
			gWindow = window;
		}



		void set_scene_manager(SceneManager* const sceneManager)
		{
			gSceneManager = sceneManager;
		}



		void set_renderer(Renderer* const renderer)
		{
			gRenderer = renderer;
		}
	}
}
