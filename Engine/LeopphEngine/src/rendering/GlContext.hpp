#pragma once

#include <glad/gl.h>


namespace leopph
{
	class OpenGlContext
	{
		protected:
			OpenGlContext();

		private:
			static void debug_message_callback(GLenum src, GLenum type, GLuint, GLenum severity, GLsizei, GLchar const* msg, void const*);
	};
}
