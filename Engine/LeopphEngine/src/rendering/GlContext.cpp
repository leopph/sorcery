#include "GlContext.hpp"

#include "Logger.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <format>
#include <string>


namespace leopph
{
	OpenGlContext::OpenGlContext()
	{
		if (gladLoadGL(glfwGetProcAddress) == 0)
		{
			Logger::get_instance().critical("Failed to initialize OpenGL context.");
			return;
		}

		if (!GLAD_GL_ARB_bindless_texture)
		{
			Logger::get_instance().critical("Failed to load required OpenGL extension [ARB_bindless_texture].");
		}

		#ifndef NDEBUG
		int major, minor;
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);

		Logger::get_instance().debug(std::format("OpenGL {}.{}", major, minor));
		Logger::get_instance().debug(std::format("{} {}", reinterpret_cast<char const*>(glGetString(GL_VENDOR)), reinterpret_cast<char const*>(glGetString(GL_RENDERER))));

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(debug_message_callback, nullptr);
		#endif
	}



	void OpenGlContext::debug_message_callback(GLenum const src, GLenum const type, GLuint const, GLenum const severity, GLsizei const, GLchar const* const msg, void const* const)
	{
		std::string source;

		switch (src)
		{
			case GL_DEBUG_SOURCE_API:
			{
				source = "API";
				break;
			}

			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
			{
				source = "WINDOW SYSTEM";
				break;
			}

			case GL_DEBUG_SOURCE_SHADER_COMPILER:
			{
				source = "SHADER COMPILER";
				break;
			}

			case GL_DEBUG_SOURCE_THIRD_PARTY:
			{
				source = "THIRD PARTY";
				break;
			}

			case GL_DEBUG_SOURCE_APPLICATION:
			{
				source = "APPLICATION";
				break;
			}

			case GL_DEBUG_SOURCE_OTHER:
			{
				source = "OTHER";
				break;
			}

			default:
			{
				return;
			}
		}

		std::string msgSeverity;

		switch (severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:
			{
				msgSeverity = "HIGH";
				break;
			}

			case GL_DEBUG_SEVERITY_MEDIUM:
			{
				msgSeverity = "MEDIUM";
				break;
			}

			case GL_DEBUG_SEVERITY_LOW:
			{
				msgSeverity = "LOW";
				break;
			}

			case GL_DEBUG_SEVERITY_NOTIFICATION:
			{
				msgSeverity = "NOTIFICATION";
				break;
			}

			default:
			{
				return;
			}
		}

		switch (type)
		{
			case GL_DEBUG_TYPE_ERROR:
			{
				Logger::get_instance().error(std::format("[OpenGL][{}][{}]: {}", source, msgSeverity, msg));
				return;
			}

			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			case GL_DEBUG_TYPE_PORTABILITY:
			case GL_DEBUG_TYPE_PERFORMANCE:
			{
				Logger::get_instance().warning(std::format("[OpenGL][{}[{}]: {}", source, msgSeverity, msg));
				return;
			}

			case GL_DEBUG_TYPE_MARKER:
			case GL_DEBUG_TYPE_PUSH_GROUP:
			case GL_DEBUG_TYPE_POP_GROUP:
			case GL_DEBUG_TYPE_OTHER:
			default:
			{
				break;
			}
		}
	}
}
