#include "GlMesh.hpp"

#include "../../math/Matrix.hpp"
#include "../../math/Vector.hpp"

#include <glad/glad.h>

#include <cstddef>
#include <utility>


namespace leopph::internal
{
	GlMesh::GlMesh(const MeshData& meshData, const unsigned instanceBuffer) :
		m_SharedData{new SharedData
			{
				.Material = meshData.Material,
				.VertexCount = meshData.Vertices.size(),
				.IndexCount = meshData.Indices.size()
			}}
	{
		glCreateVertexArrays(1, &m_SharedData->VertexArray);

		if (m_SharedData->IndexCount > 0ull)
		{
			glCreateBuffers(m_SharedData->Buffers.size(), m_SharedData->Buffers.data());
			glNamedBufferStorage(m_SharedData->Buffers[INDEX_BUFFER], meshData.Indices.size() * sizeof(unsigned), meshData.Indices.data(), 0);
			glVertexArrayElementBuffer(m_SharedData->VertexArray, m_SharedData->Buffers[INDEX_BUFFER]);
		}
		else
		{
			glCreateBuffers(1, &m_SharedData->Buffers[VERTEX_BUFFER]);
		}

		glNamedBufferStorage(m_SharedData->Buffers[VERTEX_BUFFER], meshData.Vertices.size() * sizeof(decltype(meshData.Vertices)::value_type), meshData.Vertices.data(), 0);
		glVertexArrayVertexBuffers(m_SharedData->VertexArray, 0, 2, std::array{m_SharedData->Buffers[VERTEX_BUFFER], instanceBuffer}.data(), std::array<GLintptr, 2>{0, 0}.data(), std::array<GLsizei, 2>{sizeof(decltype(meshData.Vertices)::value_type), sizeof(std::pair<Matrix4, Matrix4>)}.data());

		glVertexArrayAttribFormat(m_SharedData->VertexArray, 0, 3, GL_FLOAT, GL_FALSE, offsetof(decltype(meshData.Vertices)::value_type, Position));
		glVertexArrayAttribBinding(m_SharedData->VertexArray, 0, 0);
		glEnableVertexArrayAttrib(m_SharedData->VertexArray, 0);

		glVertexArrayAttribFormat(m_SharedData->VertexArray, 1, 3, GL_FLOAT, GL_FALSE, offsetof(decltype(meshData.Vertices)::value_type, Normal));
		glVertexArrayAttribBinding(m_SharedData->VertexArray, 1, 0);
		glEnableVertexArrayAttrib(m_SharedData->VertexArray, 1);

		glVertexArrayAttribFormat(m_SharedData->VertexArray, 2, 2, GL_FLOAT, GL_FALSE, offsetof(decltype(meshData.Vertices)::value_type, TexCoord));
		glVertexArrayAttribBinding(m_SharedData->VertexArray, 2, 0);
		glEnableVertexArrayAttrib(m_SharedData->VertexArray, 2);

		glVertexArrayAttribFormat(m_SharedData->VertexArray, 3, 4, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(m_SharedData->VertexArray, 3, 1);
		glEnableVertexArrayAttrib(m_SharedData->VertexArray, 3);

		glVertexArrayAttribFormat(m_SharedData->VertexArray, 4, 4, GL_FLOAT, GL_FALSE, sizeof(Vector4));
		glVertexArrayAttribBinding(m_SharedData->VertexArray, 4, 1);
		glEnableVertexArrayAttrib(m_SharedData->VertexArray, 4);

		glVertexArrayAttribFormat(m_SharedData->VertexArray, 5, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_SharedData->VertexArray, 5, 1);
		glEnableVertexArrayAttrib(m_SharedData->VertexArray, 5);

		glVertexArrayAttribFormat(m_SharedData->VertexArray, 6, 4, GL_FLOAT, GL_FALSE, 3 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_SharedData->VertexArray, 6, 1);
		glEnableVertexArrayAttrib(m_SharedData->VertexArray, 6);

		glVertexArrayAttribFormat(m_SharedData->VertexArray, 7, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_SharedData->VertexArray, 7, 1);
		glEnableVertexArrayAttrib(m_SharedData->VertexArray, 7);

		glVertexArrayAttribFormat(m_SharedData->VertexArray, 8, 4, GL_FLOAT, GL_FALSE, 5 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_SharedData->VertexArray, 8, 1);
		glEnableVertexArrayAttrib(m_SharedData->VertexArray, 8);

		glVertexArrayAttribFormat(m_SharedData->VertexArray, 9, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_SharedData->VertexArray, 9, 1);
		glEnableVertexArrayAttrib(m_SharedData->VertexArray, 9);

		glVertexArrayAttribFormat(m_SharedData->VertexArray, 10, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_SharedData->VertexArray, 10, 1);
		glEnableVertexArrayAttrib(m_SharedData->VertexArray, 10);

		glVertexArrayBindingDivisor(m_SharedData->VertexArray, 1, 1);
	}

	GlMesh::GlMesh(const GlMesh& other) :
		m_SharedData{other.m_SharedData}
	{
		++m_SharedData->RefCount;
	}

	GlMesh& GlMesh::operator=(const GlMesh& other)
	{
		if (this == &other)
		{
			return *this;
		}

		Deinit();
		m_SharedData = other.m_SharedData;
		++m_SharedData->RefCount;
		return *this;
	}

	GlMesh::GlMesh(GlMesh&& other) noexcept :
		GlMesh{other}
	{}

	GlMesh& GlMesh::operator=(GlMesh&& other) noexcept
	{
		return *this = other;
	}

	GlMesh::~GlMesh()
	{
		Deinit();
	}

	bool GlMesh::operator==(const GlMesh& other) const
	{
		return m_SharedData == other.m_SharedData;
	}

	void GlMesh::DrawShaded(ShaderProgram& shader, std::size_t nextFreeTextureUnit, const std::size_t instanceCount) const
	{
		shader.SetUniform("u_Material.ambientColor", static_cast<Vector3>(m_SharedData->Material->AmbientColor));
		shader.SetUniform("u_Material.diffuseColor", static_cast<Vector3>(m_SharedData->Material->DiffuseColor));
		shader.SetUniform("u_Material.specularColor", static_cast<Vector3>(m_SharedData->Material->SpecularColor));
		shader.SetUniform("u_Material.shininess", m_SharedData->Material->Shininess);

		if (m_SharedData->Material->AmbientMap != nullptr)
		{
			shader.SetUniform("u_Material.hasAmbientMap", true);
			shader.SetUniform("u_Material.ambientMap", static_cast<int>(nextFreeTextureUnit));
			glBindTextureUnit(nextFreeTextureUnit, m_SharedData->Material->AmbientMap->Id);
			++nextFreeTextureUnit;
		}
		else
		{
			shader.SetUniform("u_Material.hasAmbientMap", false);
		}

		if (m_SharedData->Material->DiffuseMap != nullptr)
		{
			shader.SetUniform("u_Material.hasDiffuseMap", true);
			shader.SetUniform("u_Material.diffuseMap", static_cast<int>(nextFreeTextureUnit));
			glBindTextureUnit(nextFreeTextureUnit, m_SharedData->Material->DiffuseMap->Id);
			++nextFreeTextureUnit;
		}
		else
		{
			shader.SetUniform("u_Material.hasDiffuseMap", false);
		}

		if (m_SharedData->Material->SpecularMap != nullptr)
		{
			shader.SetUniform("u_Material.hasSpecularMap", true);
			shader.SetUniform("u_Material.specularMap", static_cast<int>(nextFreeTextureUnit));
			glBindTextureUnit(nextFreeTextureUnit, m_SharedData->Material->SpecularMap->Id);
			++nextFreeTextureUnit;
		}
		else
		{
			shader.SetUniform("u_Material.hasSpecularMap", false);
		}

		if (!m_SharedData->Material->TwoSided)
		{
			glDisable(GL_CULL_FACE);
		}

		glBindVertexArray(m_SharedData->VertexArray);
		if (m_SharedData->IndexCount > 0ull)
		{
			glDrawElementsInstanced(GL_TRIANGLES, m_SharedData->IndexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
		}
		else
		{
			glDrawArraysInstanced(GL_TRIANGLES, 0, m_SharedData->VertexCount, instanceCount);
		}
		glBindVertexArray(0);

		if (!m_SharedData->Material->TwoSided)
		{
			glEnable(GL_CULL_FACE);
		}
	}

	void GlMesh::DrawDepth(const std::size_t instanceCount) const
	{
		if (!m_SharedData->Material->TwoSided)
		{
			glDisable(GL_CULL_FACE);
		}

		glBindVertexArray(m_SharedData->VertexArray);
		if (m_SharedData->IndexCount > 0ull)
		{
			glDrawElementsInstanced(GL_TRIANGLES, m_SharedData->IndexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
		}
		else
		{
			glDrawArraysInstanced(GL_TRIANGLES, 0, m_SharedData->VertexCount, instanceCount);
		}
		glBindVertexArray(0);

		if (!m_SharedData->Material->TwoSided)
		{
			glEnable(GL_CULL_FACE);
		}
	}

	void GlMesh::Deinit() const
	{
		--m_SharedData->RefCount;

		if (m_SharedData->RefCount == 0ull)
		{
			glDeleteBuffers(m_SharedData->Buffers.size(), m_SharedData->Buffers.data());
			glDeleteVertexArrays(1, &m_SharedData->VertexArray);
		}
	}
}
