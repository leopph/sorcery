#define GLFW_INCLUDE_NONE

#include "rendering/gl/GlCore.hpp"

#include "Logger.hpp"

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <string>
#include <format>

namespace leopph::internal::opengl
{
	namespace
	{
		auto MessageCallback(GLenum const src, GLenum const type, GLuint const, GLenum const severity, GLsizei const, GLchar const* const msg, void const* const) -> void
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
					Logger::Instance().Error(logMsg);
					return;
				}

				case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
				case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
				case GL_DEBUG_TYPE_PORTABILITY:
				case GL_DEBUG_TYPE_PERFORMANCE:
				{
					auto const logMsg{"OpenGL warning from [" + source + "] with severity [" + msgSeverity + "]: " + msg};
					Logger::Instance().Warning(logMsg);
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

	auto Init() -> void
	{
		if (gladLoadGL(glfwGetProcAddress) == 0)
		{
			auto const errMsg = "Failed to initialize OpenGL context.";
			Logger::Instance().Critical(errMsg);
			throw std::runtime_error{errMsg};
		}

		#ifndef NDEBUG
		int major, minor;
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);

		Logger::Instance().Debug("OpenGL " + std::to_string(major) + "." + std::to_string(minor));
		Logger::Instance().Debug(std::format("{} {}", reinterpret_cast<char const*>(glGetString(GL_VENDOR)), reinterpret_cast<char const*>(glGetString(GL_RENDERER))));

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(MessageCallback, nullptr);
		#endif
	}
}
