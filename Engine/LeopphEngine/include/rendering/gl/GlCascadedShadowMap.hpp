#pragma once

#include "EventReceiver.hpp"
#include "Frustum.hpp"
#include "GlFramebufferObject.hpp"
#include "GlTextureObject.hpp"
#include "Matrix.hpp"
#include "events/DirShadowEvent.hpp"
#include "rendering/shaders/ShaderProgram.hpp"

#include <cstddef>
#include <span>
#include <string_view>
#include <vector>


namespace leopph::internal
{
	class GlCascadedShadowMap final : public EventReceiver<DirShadowEvent>
	{
		public:
			struct CascadeBounds;

			GlCascadedShadowMap();

			GlCascadedShadowMap(GlCascadedShadowMap const& other) = delete;
			auto operator=(GlCascadedShadowMap const& other) -> GlCascadedShadowMap& = delete;

			GlCascadedShadowMap(GlCascadedShadowMap&& other) = delete;
			auto operator=(GlCascadedShadowMap&& other) -> GlCascadedShadowMap& = delete;

			~GlCascadedShadowMap() noexcept override = default;

			// Clears the currently bound cascade map.
			auto Clear() const -> void;

			// Binds the cascade for writing.
			auto BindForWriting(std::size_t cascadeIndex) const -> void;

			// Binds the shadow maps to consecutive texture units starting at texUnit, sets the passed uniform, and returns the next available texture unit.
			[[nodiscard]]
			auto BindForReading(ShaderProgram& shader, std::string_view uniformName, GLuint texUnit) const -> GLuint;

			// Returns the the camera-to-clip matrix for all cascades.
			[[nodiscard]] static
			auto CascadeMatrix(Frustum const& frustum, std::span<CascadeBounds const> cascadeBounds, Matrix4 const& worldTolightMat, Matrix4 const& camToLightMat, float bBoxNearOffset) -> std::span<Matrix4>;

			// Returns the Z bounds of the used shadow cascades in camera view space.
			[[nodiscard]] static
			auto CalculateCascadeBounds(float near, float far) -> std::span<CascadeBounds>;

		private:
			struct Cascade
			{
				std::size_t Resolution;
				GlTextureObject<GlTextureType::T2D> ShadowMap;
			};


			auto ConfigCascades(std::span<std::size_t const> resolutions) -> void;

			// Updates the cascades based on the new resolutions.
			auto OnEventReceived(DirShadowEvent const& event) -> void override;

			GlFramebufferObject m_Framebuffer;
			std::vector<Cascade> m_Cascades;
	};


	// Utility struct for storing info about shadow cascades
	struct GlCascadedShadowMap::CascadeBounds
	{
		// The distance of the cascade's near face from the camera.
		float Near;
		// The distance of the cascade's far face from the camera.
		float Far;
	};
}
