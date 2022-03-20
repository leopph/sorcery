#pragma once

#include "../components/Camera.hpp"
#include "../events/DirShadowEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "../math/Matrix.hpp"
#include "opengl/GlFramebuffer.hpp"
#include "opengl/GlTexture.hpp"
#include "shaders/ShaderProgram.hpp"

#include <cstddef>
#include <span>
#include <string_view>
#include <vector>


namespace leopph::internal
{
	class CascadedShadowMap final : public EventReceiver<DirShadowEvent>
	{
		public:
			struct CascadeBounds;

			CascadedShadowMap();

			CascadedShadowMap(CascadedShadowMap const& other) = delete;
			auto operator=(CascadedShadowMap const& other) -> CascadedShadowMap& = delete;

			CascadedShadowMap(CascadedShadowMap&& other) = delete;
			auto operator=(CascadedShadowMap&& other) -> CascadedShadowMap& = delete;

			~CascadedShadowMap() noexcept override = default;

			// Clears the currently bound cascade.
			auto Clear() const -> void;

			// Binds the cascade for writing.
			auto BindForWriting(std::size_t cascadeIndex) const -> void;

			// Binds the the shadow map referred to by cascadeIndex as render target and set its value to the default.
			auto BindForWritingAndClear(std::size_t cascadeIndex) const -> void;

			// Binds the shadow maps to consecutive texture units starting at texUnit, sets the passed uniform, and returns the next available texture unit.
			[[nodiscard]]
			auto BindForReading(ShaderProgram& shader, std::string_view uniformName, GLuint texUnit) const -> GLuint;

			// Returns a Matrix that defines the transformation that is used to render world space primitives to shadow maps.
			[[nodiscard]]
			auto CascadeMatrix(CascadeBounds cascadeBounds, Matrix4 const& cameraInverseMatrix, Matrix4 const& lightViewMatrix, float bBoxNearOffset) const -> Matrix4;

			// Returns the bounds of the used shadow cascades in camera view space.
			[[nodiscard]] static
			auto CalculateCascadeBounds(Camera const& cam) -> std::span<CascadeBounds>;

		private:
			struct Cascade
			{
				std::size_t Resolution;
				GlTexture<GlTextureType::T2D> ShadowMap;
			};

			auto ConfigCascades(std::span<std::size_t const> resolutions) -> void;

			// Updates the cascades based on the new resolutions.
			auto OnEventReceived(DirShadowEvent const& event) -> void override;

			GlFramebuffer m_Framebuffer;
			std::vector<Cascade> m_Cascades;
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
