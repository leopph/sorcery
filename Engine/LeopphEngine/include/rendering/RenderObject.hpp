#pragma once

#include "Matrix.hpp"
#include "RenderComponent.hpp"
#include "Types.hpp"

#include <span>
#include <vector>


namespace leopph::internal
{
	class RenderObject
	{
		public:
			struct RenderInstanceData
			{
				Matrix4 WorldTransform;
				Matrix4 NormalTransform;
				bool IsInstanced;
				bool CastsShadow;
			};


			auto RegisterRenderComponent(RenderComponent* renderComponent) -> void;
			auto UnregisterRenderComponent(RenderComponent* renderComponent) -> void;
			[[nodiscard]] auto NumRenderComponents() const -> u64;

			// Update the per component render instance cache by extracting data from the registered RenderComponents.
			// Returns the cached data.
			auto ExtractRenderInstanceData() -> std::span<RenderInstanceData const>;

			// Returns the cached set of render instance data.
			[[nodiscard]] auto GetRenderInstanceData() const -> std::span<RenderInstanceData const>;

			virtual ~RenderObject() = default;

		protected:
			RenderObject() = default;

			RenderObject(RenderObject const& other) = default;
			auto operator=(RenderObject const& other) -> RenderObject& = default;

			RenderObject(RenderObject&& other) noexcept = default;
			auto operator=(RenderObject&& other) noexcept -> RenderObject& = default;

		private:
			std::vector<RenderComponent*> m_Components;
			std::vector<RenderInstanceData> m_RenderInstancesCache; // cached render instance data
	};
}
