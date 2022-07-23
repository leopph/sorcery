#pragma once

#include "WindowImpl.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <string>


namespace leopph::internal
{
	class GlfwWindowImpl final : public WindowImpl
	{
		public:
			GlfwWindowImpl();

			~GlfwWindowImpl() override;

			[[nodiscard]]
			unsigned Width() const override;
			void Width(unsigned newWidth) override;

			[[nodiscard]]
			unsigned Height() const override;
			void Height(unsigned newHeight) override;

			[[nodiscard]]
			bool Fullscreen() const override;
			void Fullscreen(bool newValue) override;

			[[nodiscard]]
			bool Vsync() const override;
			void Vsync(bool newValue) override;

			[[nodiscard]]
			std::string_view Title() const override;
			void Title(std::string newTitle) override;

			[[nodiscard]]
			const Vector4& ClearColor() const override;
			void ClearColor(Vector4 const& color) override;

			[[nodiscard]]
			CursorState CursorMode() const override;
			void CursorMode(CursorState newState) override;

			[[nodiscard]]
			float RenderMultiplier() const override;
			void RenderMultiplier(float newMult) override;

			[[nodiscard]]
			bool ShouldClose() override;
			void ShouldClose(bool val) override;

			[[nodiscard]]
			std::vector<DisplayMode> GetSupportedDisplayModes() const override;

			void PollEvents() override;
			void SwapBuffers() override;
			void Clear() override;

			GlfwWindowImpl(GlfwWindowImpl const& other) = delete;
			GlfwWindowImpl(GlfwWindowImpl&& other) = delete;

			GlfwWindowImpl& operator=(GlfwWindowImpl const& other) = delete;
			GlfwWindowImpl& operator=(GlfwWindowImpl&& other) = delete;

		private:
			void SendWindowEvent() const;

			static void FramebufferSizeCallback(GLFWwindow*, int width, int height);
			static void KeyCallback(GLFWwindow*, int key, int, int action, int);
			static void MouseCallback(GLFWwindow*, double x, double y);

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
