#pragma once

#include "LeopphApi.hpp"
#include "Types.hpp"

#include <cstddef>
#include <filesystem>
#include <span>
#include <utility>
#include <vector>


namespace leopph
{
	class Settings
	{
		public:
			enum class GraphicsApi
			{
				OpenGl
			};

			
			// An empty location is returned if shader caching is turned off.
			[[nodiscard]] LEOPPHAPI std::filesystem::path const& get_shader_cache_path();
			LEOPPHAPI void set_shader_cache_path(std::filesystem::path path);
			[[nodiscard]] LEOPPHAPI bool is_caching_shaders() const;


			[[nodiscard]] LEOPPHAPI GraphicsApi get_current_graphics_api() const;
			LEOPPHAPI void set_graphics_api(GraphicsApi graphicsApi);


			// Get the width of the current window. This is the same as Window::get_width.
			[[nodiscard]] LEOPPHAPI u32 WindowWidth() const noexcept;


			// Set the width of the current window. This is the same as Window::get_width.
			LEOPPHAPI void WindowWidth(u32 newWidth) noexcept;


			// Get the height of the current window. This is the same as Window::Height.
			[[nodiscard]] LEOPPHAPI u32 WindowHeight() const noexcept;


			// Set the height of the current window. This is the same as Window::Height.
			LEOPPHAPI void WindowHeight(u32 newHeight) noexcept;


			// Get the current render multiplier. This is the same as Window::RenderMultiplier.
			[[nodiscard]] LEOPPHAPI f32 RenderMultiplier() const noexcept;


			// Set the render multiplier. This is the same as Window::RenderMultiplier.
			LEOPPHAPI void RenderMultiplier(f32 newMult) noexcept;


			// Get whether the current window has exclusive access to the monitor. This is the same as Window::is_fullscreen.
			[[nodiscard]] LEOPPHAPI bool Fullscreen() const noexcept;


			// Set whether the window should have exclusive access to the monitor. This is the same as Window::is_fullscreen.
			LEOPPHAPI void Fullscreen(bool newVal) noexcept;


			// Get whether Vsync is turned on.
			// This is exactly the same value as what Window::Vsync returns.
			[[nodiscard]] LEOPPHAPI bool Vsync() const;


			// Set whether Vsync is turned on.
			// This has exactly the same effect as using Window::Vsync.
			LEOPPHAPI void Vsync(bool newVal);


			// Get the gamma correction value.
			LEOPPHAPI f32 Gamma() const;


			// Set the gamma correction value.
			LEOPPHAPI void Gamma(f32 newVal);


			Settings(Settings const& other) = delete;
			Settings& operator=(Settings const& other) = delete;

			Settings(Settings&& other) noexcept = delete;
			Settings& operator=(Settings&& other) noexcept = delete;


		protected:
			Settings() noexcept = default;
			~Settings() noexcept = default;


			// Cache to store serializable window configs.
			// Needs to be kept in sync with Window::Instance!
			struct WindowSettings
			{
				u32 Width{960};
				u32 Height{540};
				f32 RenderMultiplier{1};
				bool Fullscreen{false};
				bool Vsync{false};
			} m_WindowSettingsCache;



			struct RenderingSettings
			{
				GraphicsApi Api{GraphicsApi::OpenGl};
				GraphicsApi PendingApi{Api};
				f32 Gamma{2.2f};
			} m_RenderingSettings;



			struct DirLightSettings
			{
				std::vector<u16> Res{4096, 2048, 1024};
				f32 Corr{.75f};
			} m_DirLightSettings;



			struct SpotLightSettings
			{
				u16 Res{2048};
				u8 MaxNum{8};
			} m_SpotLightSettings;



			struct PointLightSettings
			{
				u16 Res{1024};
				u8 MaxNum{8};
			} m_PointLightSettings;


			std::filesystem::path m_ShaderCache;

			// When a value changes this is set to true
			// serialization will happen at the end of the frame
			bool m_Serialize{false};

			static std::filesystem::path s_FilePath;
	};
}
