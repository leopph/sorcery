#pragma once

#include "../components/lighting/DirLight.hpp"
#include "../events/EventReceiver.hpp"
#include "../events/DirShadowMapResChangedEvent.hpp"
#include "../math/Matrix.hpp"

#include <cstddef>
#include <vector>


namespace leopph::impl
{
	class CascadedShadowMap : public EventReceiver<DirShadowMapResChangedEvent>
	{
	public:
		CascadedShadowMap();

		CascadedShadowMap(const CascadedShadowMap& other) = delete;
		CascadedShadowMap(CascadedShadowMap&& other) = delete;

		CascadedShadowMap& operator=(const CascadedShadowMap& other) = delete;
		CascadedShadowMap& operator=(CascadedShadowMap&& other) = delete;

		~CascadedShadowMap();

		void BindTextureForWriting(std::size_t cascadeIndex) const;
		void UnbindTextureFromWriting() const;

		void BindTexturesForReading(std::size_t texUnit);
		void UnbindTexturesFromReading() const;

		void Clear() const;

		Matrix4 ProjectionMatrix(std::size_t cascadeIndex, const DirectionalLight& light, const Matrix4& lightTransform) const;


	private:
		unsigned m_Fbo;
		std::vector<unsigned> m_TexIds;
		std::size_t m_TexBindStartIndex;
		std::vector<Matrix4> m_ProjMatrices;

		void OnEventReceived(const DirShadowMapResChangedEvent& event) override;
		void Init(std::vector<std::size_t> resolutions);
		void Deinit();
	};
}