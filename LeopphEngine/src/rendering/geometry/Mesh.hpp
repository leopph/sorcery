#pragma once

#include "MeshData.hpp"
#include "../shaders/ShaderProgram.hpp"


namespace leopph::impl
{
	class Mesh
	{
	public:
		explicit Mesh(MeshData& meshData);
		Mesh(const Mesh& other) = delete;
		Mesh(Mesh&& other) = delete;

		~Mesh();

		Mesh& operator=(const Mesh& other) = delete;
		Mesh& operator=(Mesh&& other) = delete;

		void DrawShaded(ShaderProgram& shader, std::size_t nextFreeTextureUnit) const;
		void DrawDepth() const;

		// Reload the Mesh by rereading data from its MeshData source.
		void Update();
	};
}