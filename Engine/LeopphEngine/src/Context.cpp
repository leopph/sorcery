#include "Context.hpp"

#include "InternalContext.hpp"
#include "SettingsImpl.hpp"
#include "windowing/WindowImpl.hpp"


namespace leopph
{
	namespace internal
	{
		namespace
		{
			WindowImpl* g_WindowImpl;
			SettingsImpl* g_SettingsImpl;
			DataManager* g_DataManager;
			Renderer* g_Renderer;
		}
	}



	Window* GetWindow()
	{
		return internal::g_WindowImpl;
	}



	Settings* GetSettings()
	{
		return internal::g_SettingsImpl;
	}



	namespace internal
	{
		WindowImpl* GetWindowImpl()
		{
			return g_WindowImpl;
		}



		SettingsImpl* GetSettingsImpl()
		{
			return g_SettingsImpl;
		}



		DataManager* GetDataManager()
		{
			return g_DataManager;
		}



		Renderer* GetRenderer()
		{
			return g_Renderer;
		}



		void SetWindowImpl(WindowImpl* window)
		{
			g_WindowImpl = window;
		}



		void SetSettingsImpl(SettingsImpl* settings)
		{
			g_SettingsImpl = settings;
		}



		void SetRenderer(Renderer* renderer)
		{
			g_Renderer = renderer;
		}



		void SetDataManager(DataManager* dataManager)
		{
			g_DataManager = dataManager;
		}
	}
}
