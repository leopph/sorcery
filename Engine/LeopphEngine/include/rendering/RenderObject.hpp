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


			void RegisterRenderComponent(RenderComponent* renderComponent);
			void UnregisterRenderComponent(RenderComponent* renderComponent);
			[[nodiscard]] u64 NumRenderComponents() const;

			// Update the per component render instance cache by extracting data from the registered RenderComponents.
			// Returns the cached data.
			std::span<RenderInstanceData const> ExtractRenderInstanceData();

			// Returns the cached set of render instance data.
			[[nodiscard]] std::span<RenderInstanceData const> GetRenderInstanceData() const;

			virtual ~RenderObject() = default;

		protected:
			RenderObject() = default;

			RenderObject(RenderObject const& other) = default;
			RenderObject& operator=(RenderObject const& other) = default;

			RenderObject(RenderObject&& other) noexcept = default;
			RenderObject& operator=(RenderObject&& other) noexcept = default;

		private:
			std::vector<RenderComponent*> m_Components;
			std::vector<RenderInstanceData> m_RenderInstancesCache; // cached render instance data
	};
}
