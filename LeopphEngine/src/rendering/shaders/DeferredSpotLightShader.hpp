#pragma once

#include "DeferredLightShader.hpp"
#include "../../components/lighting/SpotLight.hpp"
#include "../../math/Matrix.hpp"

#include <string>


namespace leopph::impl
{
	class DeferredSpotLightShader final : public DeferredLightShader
	{
		public:
			DeferredSpotLightShader();

			void SetCameraPosition(const Vector3& pos) const;
			void SetSpotLight(const SpotLight& spotLight) const;
			void SetShadowMap(int unit) const;
			void SetLightClipMatrix(const Matrix4& mat) const;


		private:
			static const std::string s_ShadowMapName;
			static const std::string s_CamPosName;
			static const std::string s_LightPosName;
			static const std::string s_LightDirName;
			static const std::string s_LightDiffName;
			static const std::string s_LightSpecName;
			static const std::string s_LightConstName;
			static const std::string s_LightLinName;
			static const std::string s_LightQuadName;
			static const std::string s_LightRangeName;
			static const std::string s_LightInAngName;
			static const std::string s_LightOutAngName;
			static const std::string s_LightClipMatName;

			const int m_ShadowMapLoc;
			const int m_CamPosLoc;
			const int m_LightPosLoc;
			const int m_LightDirLoc;
			const int m_LightDiffLoc;
			const int m_LightSpecLoc;
			const int m_LightConstLoc;
			const int m_LightLinLoc;
			const int m_LightQuadLoc;
			const int m_LightRangeLoc;
			const int m_LightInAngLoc;
			const int m_LightOutAngLoc;
			const int m_LightClipMatLoc;
	};
}