#include "OpenGl.hpp"

#include "../../util/Logger.hpp"
#include "../../util/containers/Bimap.hpp"

#include <stdexcept>
#include <string>


namespace leopph::internal::opengl
{
	namespace
	{
		Bimap<decltype(GL_VERTEX_SHADER), ShaderType,
		      #ifdef _DEBUG
		      true
		      #else
		      false
		      #endif
		> const g_ShaderTypes
		{
			{GL_VERTEX_SHADER, ShaderType::Vertex},
			{GL_GEOMETRY_SHADER, ShaderType::Geometry},
			{GL_FRAGMENT_SHADER, ShaderType::Fragment},
			{GL_COMPUTE_SHADER, ShaderType::Compute}
		};


		auto MessageCallback(GLenum const src, GLenum const type, GLuint const, GLenum const severity, GLsizei const,
		                     GLchar const* const msg, void const* const) -> void
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
		if (auto const errCode = gl3wInit(); errCode)
		{
			auto const errMsg{"Failed to initialize OpenGL."};
			Logger::Instance().Critical(errMsg);
			throw std::runtime_error{errMsg};
		}

		int major, minor;
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);
		Logger::Instance().Debug("Using OpenGL " + std::to_string(major) + "." + std::to_string(minor) + ".");
		Logger::Instance().Debug(std::string{"Using "} + reinterpret_cast<char const*>(glGetString(GL_RENDERER)) + ".");

		if (Logger::Instance().CurrentLevel() == Logger::Level::Debug)
		{
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(MessageCallback, nullptr);
		}
	}


	auto ShaderBinaryFormats() -> std::vector<GLint>
	{
		// Get the number of formats
		auto const numBinForms{
			[]
			{
				GLint ret;
				glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &ret);
				return ret;
			}()
		};

		// Get the list of formats
		std::vector<GLint> formats(numBinForms);
		glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, formats.data());
		return formats;
	}


	auto TranslateShaderType(ShaderType const type) -> decltype(GL_VERTEX_SHADER)
	{
		return g_ShaderTypes.At(type);
	}


	auto TranslateShaderType(decltype(GL_VERTEX_SHADER) const type) -> ShaderType
	{
		return g_ShaderTypes.At(type);
	}
}
