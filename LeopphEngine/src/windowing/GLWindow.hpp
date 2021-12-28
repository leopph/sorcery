#pragma once

#define GLFW_INCLUDE_NONE

#include "WindowBase.hpp"
#include "../input/KeyCode.hpp"
#include "../input/KeyState.hpp"
#include "../misc/Color.hpp"

#include <GLFW/glfw3.h>

#include <string>
#include <unordered_map>


namespace leopph::internal
{
	class GlWindow final : public WindowBase
	{
		public:
			GlWindow(int width, int height, const std::string& title, bool fullscreen);

			~GlWindow() override;

			[[nodiscard]]
			auto Width() const -> unsigned override;
			auto Width(unsigned newWidth) -> void override;

			[[nodiscard]]
			auto Height() const -> unsigned override;
			auto Height(unsigned newHeight) -> void override;

			[[nodiscard]]
			auto Fullscreen() const -> bool override;
			auto Fullscreen(bool newValue) -> void override;

			[[nodiscard]]
			auto Vsync() const -> bool override;
			auto Vsync(bool newValue) -> void override;

			[[nodiscard]]
			auto Title() const -> std::string_view override;
			auto Title(std::string newTitle) -> void override;

			[[nodiscard]]
			auto Background() const -> const Color& override;
			auto Background(const Color& color) -> void override;

			[[nodiscard]]
			auto CursorMode() const -> CursorState override;
			auto CursorMode(CursorState newState) -> void override;

			[[nodiscard]]
			auto RenderMultiplier() -> float override;
			auto RenderMultiplier(float newMult) -> void override;

			auto PollEvents() -> void override;
			auto SwapBuffers() -> void override;
			auto ShouldClose() -> bool override;
			auto Clear() -> void override;

			GlWindow(const GlWindow& other) = delete;
			GlWindow(GlWindow&& other) = delete;

			auto operator=(const GlWindow& other) -> GlWindow& = delete;
			auto operator=(GlWindow&& other) -> GlWindow& = delete;

		private:
			auto InitKeys() -> void override;

			static auto FramebufferSizeCallback(GLFWwindow*, int width, int height) -> void;
			static auto KeyCallback(GLFWwindow*, int key, int, int action, int) -> void;
			static auto MouseCallback(GLFWwindow*, double x, double y) -> void;

			const static std::unordered_map<int, KeyState> s_KeyStates;
			const static std::unordered_map<int, KeyCode> s_KeyCodes;

			GLFWwindow* m_Window;
			int m_Width;
			int m_Height;
			bool m_Fullscreen;
			bool m_Vsync;
			std::string m_Title;
			Color m_Background;
			float m_RenderMult;
	};
}
