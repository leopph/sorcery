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

			auto operator=(const OpenGLAdapter& other) -> OpenGLAdapter& = delete;
			auto operator=(OpenGLAdapter&& other) -> OpenGLAdapter& = delete;

			static auto OpenGLShaderType(ShaderType type) -> int;
			static auto AbstractShaderType(int type) -> ShaderType;

		private:
			#ifdef _DEBUG
			static const Bimap<int, ShaderType, ShaderTypeHash, ShaderTypeEqual, true> s_ShaderTypes;
			#else
			static const Bimap<int, ShaderType, ShaderTypeHash, ShaderTypeEqual> s_ShaderTypes;
			#endif
	};
}
