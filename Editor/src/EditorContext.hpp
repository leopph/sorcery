#pragma once

#include <imgui.h>
#include "ResourceStorage.hpp"
#include "Scene.hpp"
#include "ObjectFactoryManager.hpp"

#include <filesystem>
#include <atomic>

namespace leopph::editor {
class Context {
	ImGuiIO& mImGuiIo;
	ResourceStorage mResources;
	std::shared_ptr<Scene> mScene{ std::make_shared<Scene>() };
	EditorObjectFactoryManager mFactoryManager{ CreateFactoryManager() };
	Object* mSelectedObject{ nullptr };

	static inline std::filesystem::path ASSET_DIR_REL{ "Assets" };
	static inline std::filesystem::path CACHE_DIR_REL{ "Cache" };
	static inline std::filesystem::path ASSET_FILE_EXT{ ".leopphasset" };

	std::filesystem::path mProjDirAbs;
	std::filesystem::path mAssetDirAbs;
	std::filesystem::path mCacheDirAbs;

	std::atomic<bool> mBusy;

public:
	explicit Context(ImGuiIO& imGuiIO);

	[[nodiscard]] ImGuiIO const& GetImGuiIo() const noexcept;
	[[nodiscard]] ImGuiIO& GetImGuiIo() noexcept;

	[[nodiscard]] ResourceStorage const& GetResources() const noexcept;
	[[nodiscard]] ResourceStorage& GetResources() noexcept;

	[[nodiscard]] std::shared_ptr<Scene const> GetScene() const noexcept;
	[[nodiscard]] std::shared_ptr<Scene> GetScene() noexcept;

	[[nodiscard]] EditorObjectFactoryManager const& GetFactoryManager() const noexcept;
	[[nodiscard]] EditorObjectFactoryManager& GetFactoryManager() noexcept;

	[[nodiscard]] Object* GetSelectedObject() const noexcept;
	void SetSelectedObject(Object* obj) noexcept;

	[[nodiscard]] std::filesystem::path const& GetProjectDirectoryAbsolute() const noexcept;
	[[nodiscard]] std::filesystem::path const& GetAssetDirectoryAbsolute() const noexcept;
	[[nodiscard]] std::filesystem::path const& GetCacheDirectoryAbsolute() const noexcept;

	[[nodiscard]] inline static std::filesystem::path const& GetAssetFileExtension() noexcept;

	auto OpenProject(std::filesystem::path const& targetPath) -> void;

	[[nodiscard]] bool IsEditorBusy() const noexcept;

	template<typename Callable>
	auto ExecuteInBusyEditor(Callable&& callable) -> void;
};

template<typename Callable>
auto Context::ExecuteInBusyEditor(Callable&& callable) -> void {
	std::thread{
		[this, callable] {
			bool isBusy{ false };
			while (!mBusy.compare_exchange_weak(isBusy, true)) {}

			auto const oldFlags{ mImGuiIo.ConfigFlags };
			mImGuiIo.ConfigFlags |= ImGuiConfigFlags_NoMouse;
			mImGuiIo.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;
			std::invoke(callable);
			mImGuiIo.ConfigFlags = oldFlags;

			mBusy = false;
		}
	}.detach();
}

inline std::filesystem::path const& Context::GetAssetFileExtension() noexcept {
	return ASSET_FILE_EXT;
}
}
