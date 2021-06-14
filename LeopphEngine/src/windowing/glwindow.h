#pragma once

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <string>
#include <unordered_map>

#include "../api/leopphapi.h"
#include "window.h"
#include "../input/keycodes.h"
#include "../input/keystate.h"


namespace leopph::impl
{
	class GLWindowImpl : public Window
	{
	public:
		LEOPPHAPI GLWindowImpl(unsigned width, unsigned height, const std::string& title, bool fullscreen);
		LEOPPHAPI ~GLWindowImpl() override;

		LEOPPHAPI void Width(unsigned newWidth) override;
		LEOPPHAPI void Height(unsigned newHeight) override;

		LEOPPHAPI void PollEvents() override;
		LEOPPHAPI void SwapBuffers() override;
		LEOPPHAPI bool ShouldClose() override;
		LEOPPHAPI void Clear() override;

		LEOPPHAPI CursorState CursorMode() const override;
		LEOPPHAPI void CursorMode(CursorState newState) override;

	private:
		void InitKeys() override;
		
		static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void MouseCallback(GLFWwindow* window, double x, double y);

		const static std::unordered_map<int, KeyCode> s_KeyCodes;
		const static std::unordered_map<int, KeyState> s_KeyStates;

		GLFWwindow* m_Window;
	};
}