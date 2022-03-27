#include "GlMesh.hpp"

#include "../../math/Matrix.hpp"
#include "../../math/Vector.hpp"


namespace leopph::internal
{
	GlMesh::GlMesh(internal::Mesh const* mesh, GLuint const instanceBuffer) :
		m_Mesh{mesh}
	{
		glCreateVertexArrays(1, &m_VertexArray);

		// Add the instance buffer to the vertex array.
		glVertexArrayVertexBuffer(m_VertexArray, 1, instanceBuffer, 0, sizeof(std::pair<Matrix4, Matrix4>));

		// Specify attributes in Vertex Array

		// Position
		glVertexArrayAttribFormat(m_VertexArray, 0, 3, GL_FLOAT, GL_FALSE, offsetof(decltype(m_Mesh->Vertices())::value_type, Position));
		glVertexArrayAttribBinding(m_VertexArray, 0, 0);
		glEnableVertexArrayAttrib(m_VertexArray, 0);

		// Normal
		glVertexArrayAttribFormat(m_VertexArray, 1, 3, GL_FLOAT, GL_FALSE, offsetof(decltype(m_Mesh->Vertices())::value_type, Normal));
		glVertexArrayAttribBinding(m_VertexArray, 1, 0);
		glEnableVertexArrayAttrib(m_VertexArray, 1);

		// Texture coordinates
		glVertexArrayAttribFormat(m_VertexArray, 2, 2, GL_FLOAT, GL_FALSE, offsetof(decltype(m_Mesh->Vertices())::value_type, TexCoord));
		glVertexArrayAttribBinding(m_VertexArray, 2, 0);
		glEnableVertexArrayAttrib(m_VertexArray, 2);

		// Model matrix 1st row
		glVertexArrayAttribFormat(m_VertexArray, 3, 4, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(m_VertexArray, 3, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 3);

		// Model matrix 2nd row
		glVertexArrayAttribFormat(m_VertexArray, 4, 4, GL_FLOAT, GL_FALSE, sizeof(Vector4));
		glVertexArrayAttribBinding(m_VertexArray, 4, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 4);

		// Model matrix 3rd row
		glVertexArrayAttribFormat(m_VertexArray, 5, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_VertexArray, 5, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 5);

		// Model matrix 4th row
		glVertexArrayAttribFormat(m_VertexArray, 6, 4, GL_FLOAT, GL_FALSE, 3 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_VertexArray, 6, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 6);

		// Normal matrix 1st row
		glVertexArrayAttribFormat(m_VertexArray, 7, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_VertexArray, 7, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 7);

		// Normal matrix 2nd row
		glVertexArrayAttribFormat(m_VertexArray, 8, 4, GL_FLOAT, GL_FALSE, 5 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_VertexArray, 8, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 8);

		// Normal matrix 3rd row
		glVertexArrayAttribFormat(m_VertexArray, 9, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_VertexArray, 9, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 9);

		// Normal matrix 4th row
		glVertexArrayAttribFormat(m_VertexArray, 10, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(Vector4));
		glVertexArrayAttribBinding(m_VertexArray, 10, 1);
		glEnableVertexArrayAttrib(m_VertexArray, 10);

		// Set the attributes to advance once per instance.
		glVertexArrayBindingDivisor(m_VertexArray, 1, 1);

		SetupBuffers();
	}


	auto GlMesh::DrawWithMaterial(ShaderProgram& shader, GLuint nextFreeTextureUnit, GLsizei const instanceCount) const -> void
	{
		shader.SetUniform("u_Material.diffuseColor", static_cast<Vector3>(m_Mesh->Material()->DiffuseColor));
		shader.SetUniform("u_Material.specularColor", static_cast<Vector3>(m_Mesh->Material()->SpecularColor));
		shader.SetUniform("u_Material.gloss", m_Mesh->Material()->Gloss);
		shader.SetUniform("u_Material.opacity", m_Mesh->Material()->Opacity);

		if (m_Mesh->Material()->DiffuseMap != nullptr)
		{
			shader.SetUniform("u_Material.hasDiffuseMap", true);
			shader.SetUniform("u_Material.diffuseMap", static_cast<GLint>(nextFreeTextureUnit)); // cast to GLint because only glUniform1i[v] may be used to set sampler uniforms (wtf?)
			glBindTextureUnit(nextFreeTextureUnit, m_Mesh->Material()->DiffuseMap->TextureName());
			++nextFreeTextureUnit;
		}
		else
		{
			shader.SetUniform("u_Material.hasDiffuseMap", false);
		}

		if (m_Mesh->Material()->SpecularMap != nullptr)
		{
			shader.SetUniform("u_Material.hasSpecularMap", true);
			shader.SetUniform("u_Material.specularMap", static_cast<GLint>(nextFreeTextureUnit)); // cast to GLint because only glUniform1i[v] may be used to set sampler uniforms (wtf?)
			glBindTextureUnit(nextFreeTextureUnit, m_Mesh->Material()->SpecularMap->TextureName());
			++nextFreeTextureUnit;
		}
		else
		{
			shader.SetUniform("u_Material.hasSpecularMap", false);
		}

		if (m_Mesh->Material()->OpacityMap)
		{
			shader.SetUniform("u_Material.hasOpacityMap", true);
			shader.SetUniform("u_Material.opacityMap", static_cast<GLint>(nextFreeTextureUnit)); // cast to GLint because only glUniform1i[v] may be used to set sampler uniforms (wtf?)
			glBindTextureUnit(nextFreeTextureUnit, m_Mesh->Material()->OpacityMap->TextureName());
			++nextFreeTextureUnit;
		}
		else
		{
			shader.SetUniform("u_Material.hasOpacityMap", false);
		}

		DrawWithoutMaterial(instanceCount);
	}


	auto GlMesh::DrawWithoutMaterial(GLsizei const instanceCount) const -> void
	{
		if (m_Mesh->Material()->TwoSided)
		{
			glEnable(GL_CULL_FACE);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}

		glBindVertexArray(m_VertexArray);
		glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(m_Mesh->Indices().size()), GL_UNSIGNED_INT, nullptr, instanceCount);
	}


	auto GlMesh::Mesh() const -> internal::Mesh const*
	{
		return m_Mesh;
	}


	auto GlMesh::Mesh(internal::Mesh const* mesh) -> void
	{
		m_Mesh = mesh;
		SetupBuffers();
	}


	GlMesh::~GlMesh() noexcept
	{
		glDeleteVertexArrays(1, &m_VertexArray);
		glDeleteBuffers(1, &m_VertexBuffer);
		glDeleteBuffers(1, &m_IndexBuffer);
	}


	auto GlMesh::SetupBuffers() -> void
	{
		glDeleteBuffers(1, &m_VertexBuffer);
		glDeleteBuffers(1, &m_IndexBuffer);

		glCreateBuffers(1, &m_VertexBuffer);
		glCreateBuffers(1, &m_IndexBuffer);

		glNamedBufferStorage(m_VertexBuffer, m_Mesh->Vertices().size() * sizeof(decltype(m_Mesh->Vertices())::value_type), m_Mesh->Vertices().data(), 0);
		glNamedBufferStorage(m_IndexBuffer, m_Mesh->Indices().size() * sizeof(decltype(m_Mesh->Indices())::value_type), m_Mesh->Indices().data(), 0);

		glVertexArrayVertexBuffer(m_VertexArray, 0, m_VertexBuffer, 0, sizeof(decltype(m_Mesh->Vertices())::value_type));
		glVertexArrayElementBuffer(m_VertexArray, m_IndexBuffer);
	}
}
