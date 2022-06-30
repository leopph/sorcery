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


	auto GetWindow() -> Window*
	{
		return internal::g_WindowImpl;
	}


	auto GetSettings() -> Settings*
	{
		return internal::g_SettingsImpl;
	}


	namespace internal
	{
		auto GetWindowImpl() -> WindowImpl*
		{
			return g_WindowImpl;
		}


		auto GetSettingsImpl() -> SettingsImpl*
		{
			return g_SettingsImpl;
		}


		auto GetDataManager() -> DataManager*
		{
			return g_DataManager;
		}


		auto GetRenderer() -> Renderer*
		{
			return g_Renderer;
		}


		auto SetWindowImpl(WindowImpl* window) -> void
		{
			g_WindowImpl = window;
		}


		auto SetSettingsImpl(SettingsImpl* settings) -> void
		{
			g_SettingsImpl = settings;
		}


		auto SetRenderer(Renderer* renderer) -> void
		{
			g_Renderer = renderer;
		}


		auto SetDataManager(DataManager* dataManager) -> void
		{
			g_DataManager = dataManager;
		}
	}
}
