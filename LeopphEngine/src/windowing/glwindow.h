#pragma once

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <string>
#include <unordered_map>
#include "window.h"
#include "../input/keycodes.h"
#include "../input/keystate.h"


namespace leopph::impl
{
	class GLWindowImpl : public Window
	{
	public:
		GLWindowImpl(unsigned width, unsigned height, const std::string& title, bool fullscreen);
		~GLWindowImpl() override;

		void Width(unsigned newWidth) override;
		void Height(unsigned newHeight) override;

		void Vsync(bool value) override;
		bool Vsync() const override;

		void PollEvents() override;
		void SwapBuffers() override;
		bool ShouldClose() override;
		void Clear() override;

		CursorState CursorMode() const override;
		void CursorMode(CursorState newState) override;

	private:
		void InitKeys() override;
		
		static void FramebufferSizeCallback(GLFWwindow*, int width, int height);
		static void KeyCallback(GLFWwindow*, int key, int, int action, int);
		static void MouseCallback(GLFWwindow*, double x, double y);

		const static std::unordered_map<int, KeyCode> s_KeyCodes;
		const static std::unordered_map<int, KeyState> s_KeyStates;

		GLFWwindow* m_Window;
		bool m_Vsync;
	};
}