#pragma once

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <string>
#include <functional>

#include "../api/leopphapi.h"
#include "window.h"


namespace leopph::impl
{
	class GLWindowImpl : public Window
	{
	public:
		LEOPPHAPI GLWindowImpl(unsigned width, unsigned height, const std::string& title, bool fullscreen);
		LEOPPHAPI ~GLWindowImpl();

		LEOPPHAPI virtual void Width(unsigned newWidth) override;
		LEOPPHAPI virtual void Height(unsigned newHeight) override;

		LEOPPHAPI virtual void PollEvents() override;
		LEOPPHAPI virtual void SwapBuffers() override;
		LEOPPHAPI virtual bool ShouldClose() override;
		LEOPPHAPI virtual void Clear() override;

		LEOPPHAPI virtual CursorState CursorMode() const override;
		LEOPPHAPI virtual void CursorMode(CursorState newState) override;

		// TODO make these implementation agnostic
		LEOPPHAPI static void SetKeyCallback(std::function<void(int, int)> callback);
		LEOPPHAPI static void SetMouseCallback(std::function<void(float, float)> callback);


	private:
		static std::function<void(int, int)> s_KeyCallback;
		static std::function<void(float, float)> s_MouseCallback;

		// TODO make these implementation agnostic
		static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
		static void KeyCallbackManager(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void MouseCallbackManager(GLFWwindow* window, double x, double y);

		GLFWwindow* m_Window;
	};
}