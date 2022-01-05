#pragma once

#include "../components/Camera.hpp"
#include "../events/DirShadowResChangeEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "../math/Matrix.hpp"
#include "shaders/ShaderProgram.hpp"

#include <glad/glad.h>

#include <cstddef>
#include <span>
#include <string_view>
#include <vector>


namespace leopph::internal
{
	class CascadedShadowMap final : public EventReceiver<DirShadowResChangeEvent>
	{
		public:
			struct CascadeBounds;

			CascadedShadowMap();

			CascadedShadowMap(const CascadedShadowMap& other) = delete;
			auto operator=(const CascadedShadowMap& other) -> CascadedShadowMap& = delete;

			CascadedShadowMap(CascadedShadowMap&& other) = delete;
			auto operator=(CascadedShadowMap&& other) -> CascadedShadowMap& = delete;

			~CascadedShadowMap() noexcept override;

			// Binds the the shadow map referred to by cascadeIndex as render target and set its value to the default.
			auto BindForWritingAndClear(std::size_t cascadeIndex) const -> void;

			// Binds the shadow maps to consecutive texture units starting at texUnit, sets the passed uniform, and returns the next available texture unit.
			[[nodiscard]] auto BindForReading(ShaderProgram& shader, std::string_view uniformName, GLuint texUnit) const -> GLuint;

			// Returns a Matrix that defines the transformation that is used to render world space primitives to shadow maps.
			[[nodiscard]] auto CascadeMatrix(CascadeBounds cascadeBounds, const Matrix4& cameraInverseMatrix, const Matrix4& lightViewMatrix, float bBoxNearOffset) const -> Matrix4;

			// Returns the bounds of the used shadow cascades in camera view space.
			[[nodiscard]] auto CalculateCascadeBounds(const Camera& cam) const -> std::span<CascadeBounds>;

		private:
			auto OnEventReceived(const DirShadowResChangeEvent& event) -> void override;
			auto InitShadowMaps(std::span<const std::size_t> ress) -> void;
			auto DeinitShadowMaps() const -> void;

			GLuint m_Framebuffer;
			std::vector<GLuint> m_ShadowMaps;

			constexpr static GLfloat CLEAR_DEPTH{1};
	};


	// Utility struct for storing info about shadow cascades
	struct CascadedShadowMap::CascadeBounds
	{
		// The edge of the cascade closer to the camera.
		float Near;
		// The edge of the cascade farther from the camera.
		float Far;
	};
}
