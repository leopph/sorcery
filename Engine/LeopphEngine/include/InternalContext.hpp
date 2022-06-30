#pragma once


namespace leopph::internal
{
	class WindowImpl;
	class SettingsImpl;
	class DataManager;
	class Renderer;

	auto GetWindowImpl() -> WindowImpl*;
	auto GetSettingsImpl() -> SettingsImpl*;
	auto GetDataManager() -> DataManager*;
	auto GetRenderer() -> Renderer*;

	auto SetWindowImpl(WindowImpl* window) -> void;
	auto SetSettingsImpl(SettingsImpl* settings) -> void;
	auto SetRenderer(Renderer* renderer) -> void;
	auto SetDataManager(DataManager* dataManager) -> void;
}
