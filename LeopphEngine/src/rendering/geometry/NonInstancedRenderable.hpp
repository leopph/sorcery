#pragma once

#include "ModelData.hpp"
#include "Mesh.hpp"
#include "Renderable.hpp"
#include "../shaders/ShaderProgram.hpp"

#include <memory>
#include <vector>


namespace leopph::impl
{
	class NonInstancedRenderable : public Renderable
	{
	public:
		explicit NonInstancedRenderable(ModelData& modelData);
		NonInstancedRenderable(const NonInstancedRenderable& other) = delete;
		NonInstancedRenderable(NonInstancedRenderable&& other) = delete;

		~NonInstancedRenderable() override = default;

		NonInstancedRenderable& operator=(const NonInstancedRenderable& other) = delete;
		NonInstancedRenderable& operator=(NonInstancedRenderable&& other) = delete;

		void DrawShaded(ShaderProgram& shader, std::size_t nextFreeTextureUnit) const override;
		void DrawDepth() const override;

		void Update() override;


	private:
		std::vector<std::unique_ptr<Mesh>> m_Meshes;
	};
}