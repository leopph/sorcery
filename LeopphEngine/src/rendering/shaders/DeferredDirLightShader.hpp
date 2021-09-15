#pragma once

#include "Shader.hpp"
#include "../../components/lighting/DirLight.hpp"
#include "../../math/Vector.hpp"

#include <string>



namespace leopph::impl
{
	class DeferredDirLightShader final : public Shader
	{
		public:
			DeferredDirLightShader();

			void SetPositionTexture(int unit) const;
			void SetNormalTexture(int unit) const;
			void SetDiffuseTexture(int unit) const;
			void SetSpecularTexture(int unit) const;
			void SetShineTexture(int unit) const;
			void SetCameraPosition(const Vector3& pos) const;
			void SetDirLight(const DirectionalLight& dirLight) const;
			void SetCascadeCount(unsigned count) const;
			void SetShadowMaps(const std::vector<int>& shadowMaps) const;
			void SetCascadeFarBounds(const std::vector<float>& bounds) const;
			void SetLightClipMatrices(const std::vector<Matrix4>& mats) const;


		private:
			const static std::string s_PosTexName;
			const static std::string s_NormTexName;
			const static std::string s_DiffTexName;
			const static std::string s_SpecTexName;
			const static std::string s_ShineTexName;
			const static std::string s_ShadowMapArrName;
			const static std::string s_CamPosName;
			const static std::string s_DirLightDirName;
			const static std::string s_DirLightDiffName;
			const static std::string s_DirLightSpecName;
			const static std::string s_CascCountName;
			const static std::string s_ClipMatName;
			const static std::string s_CascBoundsName;

			const int m_PosTexLoc;
			const int m_NormTexLoc;
			const int m_DiffTexLoc;
			const int m_SpecTexLoc;
			const int m_ShineTexLoc;
			const int m_ShadowMapArrLoc;
			const int m_CamPosLoc;
			const int m_DirLightDirLoc;
			const int m_DirLightDiffLoc;
			const int m_DirLightSpecLoc;
			const int m_CascCountLoc;
			const int m_ClipMatLoc;
			const int m_CascBoundsLoc;
	};
}
