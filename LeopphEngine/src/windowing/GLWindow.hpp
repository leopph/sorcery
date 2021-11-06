#pragma once

#define GLFW_INCLUDE_NONE

#include "WindowBase.hpp"
#include "../input/keycodes.h"
#include "../input/keystate.h"
#include "../misc/Color.hpp"

#include <GLFW/glfw3.h>

#include <string>
#include <unordered_map>


namespace leopph::impl
{
	class GlWindow final : public WindowBase
	{
		public:
			GlWindow(int width, int height, const std::string& title, bool fullscreen);

			~GlWindow() override;

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
			const Color& Background() const override;
			void Background(const Color& color) override;

			[[nodiscard]]
			CursorState CursorMode() const override;
			void CursorMode(CursorState newState) override;

			[[nodiscard]]
			float RenderResolution() override;
			void RenderResolution(float newMult) override;

			void PollEvents() override;
			void SwapBuffers() override;
			bool ShouldClose() override;
			void Clear() override;


			GlWindow(const GlWindow& other) = delete;
			GlWindow(GlWindow&& other) = delete;

			GlWindow& operator=(const GlWindow& other) = delete;
			GlWindow& operator=(GlWindow&& other) = delete;


		private:
			void InitKeys() override;
			void InternalSetWidth(int newWidth);
			void InternalSetHeight(int newHeight);

			static void FramebufferSizeCallback(GLFWwindow*, int width, int height);
			static void KeyCallback(GLFWwindow*, int key, int, int action, int);
			static void MouseCallback(GLFWwindow*, double x, double y);

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
