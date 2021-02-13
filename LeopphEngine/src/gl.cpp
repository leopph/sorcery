#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "gl.h"
#include "instancedata.h"

namespace leopph::implementation
{
	bool InitGL()
	{
		return gladLoadGL();
	}

	void TerminateGL()
	{
		InstanceData::DestroyAll();
		glfwTerminate();
	}

	void GetErrorGL()
	{
		GLenum error;
		while ((error = glGetError()) != GL_NO_ERROR)
			std::cout << error << std::endl;
	}
}