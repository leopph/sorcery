#pragma once


namespace leopph::internal
{
	class WindowImpl;
	class SettingsImpl;
	class DataManager;
	class Renderer;

	WindowImpl* GetWindowImpl();
	SettingsImpl* GetSettingsImpl();
	DataManager* GetDataManager();
	Renderer* GetRenderer();

	void SetWindowImpl(WindowImpl* window);
	void SetSettingsImpl(SettingsImpl* settings);
	void SetRenderer(Renderer* renderer);
	void SetDataManager(DataManager* dataManager);
}
