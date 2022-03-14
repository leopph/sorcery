#pragma once

#include "WindowImpl.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <string>


namespace leopph::internal
{
	class GlWindow final : public WindowImpl
	{
		public:
			GlWindow();

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
			auto ClearColor() const -> const Vector4& override;
			auto ClearColor(const Vector4& color) -> void override;

			[[nodiscard]]
			auto CursorMode() const -> CursorState override;
			auto CursorMode(CursorState newState) -> void override;

			[[nodiscard]]
			auto RenderMultiplier() const -> float override;
			auto RenderMultiplier(float newMult) -> void override;

			[[nodiscard]]
			auto ShouldClose() -> bool override;
			auto ShouldClose(bool val) -> void override;

			auto PollEvents() -> void override;
			auto SwapBuffers() -> void override;
			auto Clear() -> void override;

			GlWindow(const GlWindow& other) = delete;
			GlWindow(GlWindow&& other) = delete;

			auto operator=(const GlWindow& other) -> GlWindow& = delete;
			auto operator=(GlWindow&& other) -> GlWindow& = delete;

		private:
			auto OnEventReceived(EventParamType event) -> void override;

			static auto FramebufferSizeCallback(GLFWwindow* window, int width, int height) -> void;
			static auto KeyCallback(GLFWwindow*, int key, int, int action, int) -> void;
			static auto MouseCallback(GLFWwindow*, double x, double y) -> void;

			GLFWwindow* m_Window;
			int m_Width;
			int m_Height;
			bool m_Fullscreen;
			bool m_Vsync;
			std::string m_Title;
			Vector4 m_ClrColor{0, 0, 0, 1};
			float m_RenderMult;
	};
}
