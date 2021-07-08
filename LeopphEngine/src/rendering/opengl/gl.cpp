#include "gl.h"

#include "../../util/logger.h"

#include <glad/glad.h>

#include <iostream>
#include <string>

namespace leopph::impl
{
	namespace
	{
		void MessageCallback(GLenum src, GLenum type, GLuint id, GLenum severity, GLsizei length,
			const GLchar* msg, const void* userParam)
		{
			std::string source;
			switch (src)
			{
			case GL_DEBUG_SOURCE_API:
				source = "API";
				break;

			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
				source = "WINDOWING";
				break;

			case GL_DEBUG_SOURCE_SHADER_COMPILER:
				source = "SHADER COMPILATION";
				break;
				
			case GL_DEBUG_SOURCE_THIRD_PARTY:
				source = "THIRD PARTY";
				break;

			case GL_DEBUG_SOURCE_APPLICATION:
				source = "APPLICATION";
				break;

			case GL_DEBUG_SOURCE_OTHER:
				source = "OTHER";
				break;
			}

			std::string msgSeverity;
			switch (severity)
			{
			case GL_DEBUG_SEVERITY_HIGH:
				msgSeverity = "HIGH";
				break;

			case GL_DEBUG_SEVERITY_MEDIUM:
				msgSeverity = "MEDIUM";
				break;

			case GL_DEBUG_SEVERITY_LOW:
				msgSeverity = "LOW";
				break;

			case GL_DEBUG_SEVERITY_NOTIFICATION:
				return;
			}

			switch (type)
			{
			case GL_DEBUG_TYPE_ERROR:
			{
				auto logMsg{ "OpenGL error from [" + source + "] with severity [" + msgSeverity + "]: " + msg };
				Logger::Instance().Error(logMsg);
				return;
			}

			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			case GL_DEBUG_TYPE_PORTABILITY:
			case GL_DEBUG_TYPE_PERFORMANCE:
			{
				auto logMsg{ "OpenGL warning from [" + source + "] with severity [" + msgSeverity + "]: " + msg };
				Logger::Instance().Warning(logMsg);
				return;
			}

			case GL_DEBUG_TYPE_MARKER:
			case GL_DEBUG_TYPE_PUSH_GROUP:
			case GL_DEBUG_TYPE_POP_GROUP:
			case GL_DEBUG_TYPE_OTHER:
				return;
			}
		}
	}

	bool InitGL()
	{
		auto ret = gladLoadGL();

		if (Logger::Instance().CurrentLevel() == Logger::Level::DEBUG && ret)
		{
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(MessageCallback, nullptr);
		}

		return ret;
	}
}