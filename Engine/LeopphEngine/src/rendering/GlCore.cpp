#define GLFW_INCLUDE_NONE

#include "GlCore.hpp"

#include "Logger.hpp"

#include <GLFW/glfw3.h>

#include <format>
#include <stdexcept>
#include <string>


namespace leopph::internal::opengl
{
	namespace
	{
		void MessageCallback(GLenum const src, GLenum const type, GLuint const, GLenum const severity, GLsizei const, GLchar const* const msg, void const* const)
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

				default:
					source = "UNKNOWN";
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

				default:
					break;
			}

			switch (type)
			{
				case GL_DEBUG_TYPE_ERROR:
				{
					auto const logMsg{"OpenGL error from [" + source + "] with severity [" + msgSeverity + "]: " + msg};
					Logger::get_instance().error(logMsg);
					return;
				}

				case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
				case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
				case GL_DEBUG_TYPE_PORTABILITY:
				case GL_DEBUG_TYPE_PERFORMANCE:
				{
					auto const logMsg{"OpenGL warning from [" + source + "] with severity [" + msgSeverity + "]: " + msg};
					Logger::get_instance().warning(logMsg);
					return;
				}

				case GL_DEBUG_TYPE_MARKER:
				case GL_DEBUG_TYPE_PUSH_GROUP:
				case GL_DEBUG_TYPE_POP_GROUP:
				case GL_DEBUG_TYPE_OTHER:
				default:
					break;
			}
		}
	}


	void Init()
	{
		if (gladLoadGL(glfwGetProcAddress) == 0)
		{
			auto const errMsg = "Failed to initialize OpenGL context.";
			Logger::get_instance().critical(errMsg);
			throw std::runtime_error{errMsg};
		}

		#ifndef NDEBUG
		int major, minor;
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);

		Logger::get_instance().debug("OpenGL " + std::to_string(major) + "." + std::to_string(minor));
		Logger::get_instance().debug(std::format("{} {}", reinterpret_cast<char const*>(glGetString(GL_VENDOR)), reinterpret_cast<char const*>(glGetString(GL_RENDERER))));

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(MessageCallback, nullptr);
		#endif
	}
}
