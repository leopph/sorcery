#include "ShaderStage.hpp"

#include "ShaderProgram.hpp"
#include "../../util/logger.h"
#include "../../util/interface/OpenGLAdapter.hpp"

#include <glad/glad.h>

#include <algorithm>
#include <array>
#include <sstream>
#include <utility>




namespace leopph::impl
{
	ShaderStage::ShaderStage(const ShaderStageInfo info) :
		m_Name{glCreateShader(OpenGLAdapter::OpenGLShaderType(info.type))}
	{
		const auto procSrc{PreProcess(info.src, info.flags)};

		if (!procSrc.has_value())
		{
			Logger::Instance().Error("Cannot parse shader: source is empty.");
			return;
		}

		Compile(procSrc.value());

		if (const auto errMsg{CheckForCompilationErrors()};
			errMsg.has_value())
		{
			Logger::Instance().Error(errMsg.value());
			return;
		}
	}


	ShaderStage::ShaderStage(ShaderStage&& other) noexcept :
		m_Name{other.m_Name}
	{
		other.m_Name = 0;
	}



	ShaderStage::~ShaderStage()
	{
		glDeleteShader(m_Name);
	}


	ShaderStage& ShaderStage::operator=(ShaderStage&& other) noexcept
	{
		glDeleteShader(m_Name);
		m_Name = other.m_Name;
		other.m_Name = 0;
		return *this;
	}



	void ShaderStage::AttachTo(const ShaderProgram& program) const
	{
		glAttachShader(program.Name, m_Name);
	}


	std::optional<std::string> ShaderStage::PreProcess(const std::string& src, std::vector<std::string> flags) const
	{
		std::istringstream inStream{src};
		std::vector<std::string> lines;

		for (std::string line; std::getline(inStream, line);)
		{
			if (!line.empty() && !(line.find_first_not_of(' ') == std::string::npos) && !(line.find_first_not_of('\t') == std::string::npos))
			{
				lines.push_back(std::move(line));
			}
		}

		if (lines.empty())
		{
			return {};
		}

		for (std::size_t i = 0; i < flags.size(); ++i)
		{
			lines.emplace(lines.begin() + 1 + static_cast<long long>(i), "#define " + std::move(flags[i]));
		}

		std::string procSrc;
		std::ranges::for_each(lines, [&](auto& line)
		{
			procSrc += std::move(line) + '\n';
		});

		return {procSrc};
	}


	void ShaderStage::Compile(const std::string_view src) const
	{
		glShaderSource(m_Name, 1, std::array{src.data()}.data(), nullptr);
		glCompileShader(m_Name);
	}


	std::optional<std::string> ShaderStage::CheckForCompilationErrors() const
	{
		GLint status;
		glGetShaderiv(m_Name, GL_COMPILE_STATUS, &status);

		if (status == GL_FALSE)
		{
			GLint logLength;
			glGetShaderiv(m_Name, GL_INFO_LOG_LENGTH, &logLength);

			std::string errMsg;
			errMsg.resize(logLength);
			glGetShaderInfoLog(m_Name, logLength, &logLength, errMsg.data());
			return {errMsg};
		}

		return {};
	}

}
