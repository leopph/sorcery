#include "OpenGLAdapter.hpp"

#include <glad/glad.h>


namespace leopph::impl
{
#ifdef _DEBUG
	const Bimap<int, ShaderType, ShaderTypeHash, ShaderTypeEqual, true> OpenGLAdapter::s_ShaderTypes
#else
	const Bimap<int, ShaderType, ShaderTypeHash, ShaderTypeEqual> OpenGLAdapter::s_ShaderTypes;
#endif
	{
		{GL_VERTEX_SHADER, ShaderType::Vertex},
		{GL_GEOMETRY_SHADER, ShaderType::Geometry},
		{GL_FRAGMENT_SHADER, ShaderType::Fragment},
		{GL_COMPUTE_SHADER, ShaderType::Compute}
	};


	int OpenGLAdapter::OpenGLShaderType(const ShaderType type)
	{
		return s_ShaderTypes.At(type);
	}


	ShaderType OpenGLAdapter::AbstractShaderType(const int type)
	{
		return s_ShaderTypes.At(type);
	}

}