#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "gl.h"

namespace leopph
{
	bool InitGL()
	{
		return gladLoadGL();
	}

	void TerminateGL()
	{
		glfwTerminate();
	}
}