#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "gl.h"
#include "../../instances/instanceholder.h"

namespace leopph::impl
{
	bool InitGL()
	{
		return gladLoadGL();
	}

	void TerminateGL()
	{
		InstanceHolder::DestroyAll();
		glfwTerminate();
	}

	void GetErrorGL()
	{
		GLenum error;
		while ((error = glGetError()) != GL_NO_ERROR)
			std::cout << error << std::endl;
	}
}