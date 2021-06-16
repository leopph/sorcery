#include <glad/glad.h>
#include <iostream>
#include "gl.h"

namespace leopph::impl
{
	bool InitGL()
	{
		return gladLoadGL();
	}

	void GetErrorGL()
	{
		GLenum error;
		while ((error = glGetError()) != GL_NO_ERROR)
			std::cerr << error << std::endl;
	}
}