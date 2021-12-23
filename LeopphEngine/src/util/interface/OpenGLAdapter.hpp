#pragma once

#include "../../rendering/shaders/ShaderType.hpp"
#include "../../util/equal/ShaderTypeEqual.hpp"
#include "../../util/hash/ShaderTypeHash.hpp"
#include "../containers/Bimap.hpp"



namespace leopph::internal
{
	class OpenGLAdapter final
	{
		public:
			OpenGLAdapter() = delete;
			OpenGLAdapter(const OpenGLAdapter& other) = delete;
			OpenGLAdapter(OpenGLAdapter&& other) = delete;

			~OpenGLAdapter() = delete;

			OpenGLAdapter& operator=(const OpenGLAdapter& other) = delete;
			OpenGLAdapter& operator=(OpenGLAdapter&& other) = delete;


			static int OpenGLShaderType(ShaderType type);
			static ShaderType AbstractShaderType(int type);


		private:
		#ifdef _DEBUG
			static const Bimap<int, ShaderType, ShaderTypeHash, ShaderTypeEqual, true> s_ShaderTypes;
		#else
			static const Bimap<int, ShaderType, ShaderTypeHash, ShaderTypeEqual> s_ShaderTypes;
		#endif
	};
}
