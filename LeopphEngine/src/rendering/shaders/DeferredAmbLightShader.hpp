#pragma once

#include "DeferredLightShader.hpp"
#include "../../math/Vector.hpp"

#include <string>


namespace leopph::impl
{
	class DeferredAmbLightShader final : public DeferredLightShader
	{
		public:
			DeferredAmbLightShader();

			void SetAmbientLight(const Vector3& ambientLight) const;


		private:
			static const std::string s_AmbLightName;

			const int m_AmbLightLoc;
	};
}