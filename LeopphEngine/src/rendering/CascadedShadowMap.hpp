#pragma once

#include "../events/DirShadowResolutionEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "../math/Matrix.hpp"
#include "../math/Vector.hpp"
#include "shaders/ShaderProgram.hpp"

#include <cstddef>
#include <vector>


namespace leopph::internal
{
	class CascadedShadowMap final : public EventReceiver<DirShadowResolutionEvent>
	{
		public:
			CascadedShadowMap();

			CascadedShadowMap(const CascadedShadowMap& other) = delete;
			CascadedShadowMap(CascadedShadowMap&& other) = delete;

			auto operator=(const CascadedShadowMap& other) -> CascadedShadowMap& = delete;
			auto operator=(CascadedShadowMap&& other) -> CascadedShadowMap& = delete;

			~CascadedShadowMap() override;

			auto BindForWriting(std::size_t cascadeIndex) const -> void;
			auto UnbindFromWriting() const -> void;

			// Returns the next available texture unit after binding
			[[nodiscard]] auto BindForReading(ShaderProgram& shader, int texUnit) -> int;
			auto UnbindFromReading() const -> void;

			auto Clear() const -> void;

			[[nodiscard]] auto WorldToClipMatrix(std::size_t cascadeIndex, const Matrix4& cameraInverseMatrix, const Matrix4& lightViewMatrix) const -> Matrix4;
			[[nodiscard]] auto CascadeBoundsViewSpace(std::size_t cascadeIndex) const -> Vector2;

		private:
			unsigned m_Fbo;
			std::vector<unsigned> m_TexIds;
			int m_TexBindStartIndex;
			std::vector<Matrix4> m_ProjMatrices;

			auto OnEventReceived(const DirShadowResolutionEvent& event) -> void override;
			auto Init(const std::vector<std::size_t>& resolutions) -> void;
			auto Deinit() -> void;
	};
}
