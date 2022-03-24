#include "OpenGl.hpp"

#include "../../util/Logger.hpp"
#include "../../util/containers/Bimap.hpp"
#include "../../util/equal/ShaderTypeEqual.hpp"
#include "../../util/hash/ShaderTypeHash.hpp"

#include <GL/gl3w.h>

#include <stdexcept>
#include <string>


namespace leopph::internal::opengl
{
	namespace
	{
		#ifdef _DEBUG
		const Bimap<int, ShaderType, ShaderTypeHash, ShaderTypeEqual, true> s_ShaderTypes
			#else
		const Bimap<int, ShaderType, ShaderTypeHash, ShaderTypeEqual> s_ShaderTypes
			#endif
			{
				{GL_VERTEX_SHADER, ShaderType::Vertex},
				{GL_GEOMETRY_SHADER, ShaderType::Geometry},
				{GL_FRAGMENT_SHADER, ShaderType::Fragment},
				{GL_COMPUTE_SHADER, ShaderType::Compute}
			};


		auto MessageCallback(const GLenum src, const GLenum type, const GLuint, const GLenum severity, const GLsizei,
		                     const GLchar* const msg, const void* const) -> void
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
					const auto logMsg{"OpenGL error from [" + source + "] with severity [" + msgSeverity + "]: " + msg};
					Logger::Instance().Error(logMsg);
					return;
				}

				case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
				case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
				case GL_DEBUG_TYPE_PORTABILITY:
				case GL_DEBUG_TYPE_PERFORMANCE:
				{
					const auto logMsg{"OpenGL warning from [" + source + "] with severity [" + msgSeverity + "]: " + msg};
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
		if (const auto errCode = gl3wInit(); errCode)
		{
			const auto errMsg{"Failed to initialize OpenGL."};
			Logger::Instance().Critical(errMsg);
			throw std::runtime_error{errMsg};
		}

		int major, minor;
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);
		Logger::Instance().Debug("Using OpenGL " + std::to_string(major) + "." + std::to_string(minor) + ".");
		Logger::Instance().Debug(std::string{"Using "} + reinterpret_cast<const char*>(glGetString(GL_RENDERER)) + ".");

		if (Logger::Instance().CurrentLevel() == Logger::Level::Debug)
		{
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(MessageCallback, nullptr);
		}
	}


	auto ShaderBinaryFormats() -> std::vector<int>
	{
		// Get the number of formats
		const auto numBinForms{
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


	auto TranslateShaderType(const ShaderType type) -> int
	{
		return s_ShaderTypes.At(type);
	}


	auto TranslateShaderType(const int type) -> ShaderType
	{
		return s_ShaderTypes.At(type);
	}
}
