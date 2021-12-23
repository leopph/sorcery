#pragma once

#include "../events/PointShadowResolutionEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "../rendering/shaders/ShaderProgram.hpp"

#include <cstddef>


namespace leopph::internal
{
	class CubeShadowMap final : EventReceiver<PointShadowResolutionEvent>
	{
		public:
			CubeShadowMap();

			CubeShadowMap(const CubeShadowMap& other) = delete;
			CubeShadowMap(CubeShadowMap&& other) = delete;

			auto operator=(const CubeShadowMap& other) -> CubeShadowMap& = delete;
			auto operator=(CubeShadowMap&& other) -> CubeShadowMap& = delete;

			~CubeShadowMap() override;

			auto BindForWriting() const -> void;
			auto UnbindFromWriting() const -> void;

			[[nodiscard]] auto BindForReading(ShaderProgram& shader, int texUnit) -> int;
			auto UnbindFromReading() const -> void;

			auto Clear() const -> void;

		private:
			unsigned m_FrameBufferName;
			unsigned m_CubeMapName;
			int m_BoundTexUnit;

			auto OnEventReceived(EventParamType event) -> void override;

			auto Init(std::size_t resolution) -> void;
			auto Deinit() const -> void;
	};
}
