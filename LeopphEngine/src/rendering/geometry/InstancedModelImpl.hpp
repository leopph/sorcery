#pragma once

#include "InstancedMesh.hpp"
#include "ModelData.hpp"
#include "Renderable.hpp"
#include "../shaders/ShaderProgram.hpp"
#include "../../math/Matrix.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <utility>
#include <vector>


namespace leopph::impl
{
	class InstancedModelImpl : public Renderable
	{
		public:
			explicit InstancedModelImpl(ModelData& modelData);
			InstancedModelImpl(const InstancedModelImpl& other) = delete;
			InstancedModelImpl(InstancedModelImpl&& other) = delete;

			~InstancedModelImpl() override = default;

			InstancedModelImpl& operator=(const InstancedModelImpl& other) = delete;
			InstancedModelImpl& operator=(InstancedModelImpl&& other) = delete;

			void DrawShaded(ShaderProgram& shader, std::size_t nextFreeTextureUnit) const override;
			void DrawDepth() const override;

			void Update() override;

			// Loads the passed instance matrices in the instance buffer. Needs to be called before rendering.
			void SetInstanceData(const std::vector<std::pair<Matrix4, Matrix4>>& instanceMatrices) const;


		private:
			std::vector<std::unique_ptr<InstancedMesh>> m_Meshes;

			unsigned m_InstanceBuffer;
			mutable std::size_t m_InstanceBufferSize;
			mutable std::size_t m_InstanceCount;
	};
}
