#pragma once

#include "../events/SpotShadowResolutionEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "shaders/ShaderProgram.hpp"

#include <cstddef>


namespace leopph::internal
{
	class SpotLightShadowMap final : EventReceiver<SpotShadowResolutionEvent>
	{
		public:
			SpotLightShadowMap();

			SpotLightShadowMap(const SpotLightShadowMap&) = delete;
			SpotLightShadowMap(SpotLightShadowMap&& other) noexcept;

			~SpotLightShadowMap() override;

			auto operator=(const SpotLightShadowMap&) -> void = delete;
			auto operator=(SpotLightShadowMap&& other) noexcept -> SpotLightShadowMap&;

			auto BindForWriting() const -> void;
			auto UnbindFromWriting() const -> void;

			[[nodiscard]] auto BindForReading(ShaderProgram& shader, int textureUnit) const -> int;
			auto UnbindFromReading() const -> void;

			auto Clear() const -> void;

			const unsigned& Id;

		private:
			auto Init() -> void;
			auto Deinit() const -> void;
			auto OnEventReceived(EventParamType event) -> void override;

			unsigned m_Fbo;
			unsigned m_DepthTex;
			mutable int m_CurrentBindIndex;
			std::size_t m_Resolution;
	};
}
