#include "OpenGl.hpp"

#include "../../util/logger.h"

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <string>


namespace leopph::internal
{
	namespace
	{
		#pragma warning (disable: 4100)
		auto MessageCallback(GLenum src, GLenum type, GLuint id, GLenum severity, GLsizei length,
		                     const GLchar* msg, const void* userParam) -> void
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
					auto logMsg{"OpenGL error from [" + source + "] with severity [" + msgSeverity + "]: " + msg};
					Logger::Instance().Error(logMsg);
					return;
				}

				case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
				case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
				case GL_DEBUG_TYPE_PORTABILITY:
				case GL_DEBUG_TYPE_PERFORMANCE:
				{
					auto logMsg{"OpenGL warning from [" + source + "] with severity [" + msgSeverity + "]: " + msg};
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
	#pragma warning(default: 4100)

	auto InitGL() -> bool
	{
		auto ret = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));

		if (ret)
		{
			int major, minor;
			glGetIntegerv(GL_MAJOR_VERSION, &major);
			glGetIntegerv(GL_MINOR_VERSION, &minor);
			Logger::Instance().Debug("Using OpenGL " + std::to_string(major) + "." + std::to_string(minor) + ".");
			Logger::Instance().Debug(std::string{"Using renderer ["} + reinterpret_cast<const char*>(glGetString(GL_RENDERER)) + "].");
		}

		if (Logger::Instance().CurrentLevel() == Logger::Level::DEBUG && ret)
		{
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(MessageCallback, nullptr);
		}

		return ret;
	}


	auto GlShaderBinaryFormats() -> std::vector<int>
	{
		// Get the number of formats
		const auto numBinForms{[]
		{
			GLint ret;
			glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &ret);
			return ret;
		}()};

		// Get the list of formats
		std::vector<GLint> formats(numBinForms);
		glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, formats.data());
		return formats;
	}
}
