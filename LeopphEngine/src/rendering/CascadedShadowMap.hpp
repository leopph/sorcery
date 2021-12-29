#pragma once

#include "../components/Camera.hpp"
#include "../events/DirCascadeChangeEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "../math/Matrix.hpp"
#include "../math/Vector.hpp"
#include "../misc/ShadowCascade.hpp"
#include "shaders/ShaderProgram.hpp"

#include <glad/glad.h>

#include <cstddef>
#include <span>
#include <vector>


namespace leopph::internal
{
	class CascadedShadowMap final : public EventReceiver<DirCascadeChangeEvent>
	{
		public:
			CascadedShadowMap();

			CascadedShadowMap(const CascadedShadowMap& other) = delete;
			CascadedShadowMap(CascadedShadowMap&& other) = delete;

			auto operator=(const CascadedShadowMap& other) -> CascadedShadowMap& = delete;
			auto operator=(CascadedShadowMap&& other) -> CascadedShadowMap& = delete;

			~CascadedShadowMap() noexcept override;

			auto BindForWriting(std::size_t cascadeIndex) const -> void;
			static auto UnbindFromWriting() -> void;

			// Returns the next available texture unit after binding
			[[nodiscard]] auto BindForReading(ShaderProgram& shader, int texUnit) -> int;
			auto UnbindFromReading() const -> void;

			auto Clear() const -> void;

			[[nodiscard]] auto WorldToClipMatrix(std::size_t cascadeIndex, const Matrix4& cameraInverseMatrix, const Matrix4& lightViewMatrix) const -> Matrix4;
			[[nodiscard]] auto CascadeBoundsViewSpace(std::size_t cascadeIndex) const -> Vector2;

		private:
			auto OnEventReceived(const DirCascadeChangeEvent& event) -> void override;
			auto Init(std::span<const ShadowCascade> cascades) -> void;
			auto Deinit() const -> void;

			GLuint m_Framebuffer;
			std::vector<GLuint> m_Textures;
			int m_TexBindStartIndex;
			std::vector<Matrix4> m_ProjMatrices;
	};
}
