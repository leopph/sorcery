#include "GlMesh.hpp"

#include "../../math/Matrix.hpp"
#include "../../math/Vector.hpp"


namespace leopph::internal
{
	GlMesh::GlMesh(Mesh const& mesh, GLuint const instanceBuffer) :
		m_NumIndices{static_cast<GLsizei>(mesh.Indices().size())},
		m_Material{mesh.Material()}
	{
		// Add the instance buffer to the vertex array.
		glVertexArrayVertexBuffer(m_VertexArray, 1, instanceBuffer, 0, sizeof(std::pair<Matrix4, Matrix4>));

		// Specify attributes in Vertex Array

		// Position
		glVertexArrayAttribFormat(m_VertexArray, 0, 3, GL_FLOAT, GL_FALSE, offsetof(decltype(mesh.Vertices())::value_type, Position));
		glVertexArrayAttribBinding(m_VertexArray, 0, 0);
		glEnableVertexArrayAttrib(m_VertexArray, 0);

		// Normal
		glVertexArrayAttribFormat(m_VertexArray, 1, 3, GL_FLOAT, GL_FALSE, offsetof(decltype(mesh.Vertices())::value_type, Normal));
		glVertexArrayAttribBinding(m_VertexArray, 1, 0);
		glEnableVertexArrayAttrib(m_VertexArray, 1);

		// Texture coordinates
		glVertexArrayAttribFormat(m_VertexArray, 2, 2, GL_FLOAT, GL_FALSE, offsetof(decltype(mesh.Vertices())::value_type, TexCoord));
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

		glNamedBufferStorage(m_VertexBuffer, mesh.Vertices().size() * sizeof(decltype(mesh.Vertices())::value_type), mesh.Vertices().data(), 0);
		glNamedBufferStorage(m_IndexBuffer, mesh.Indices().size() * sizeof(decltype(mesh.Indices())::value_type), mesh.Indices().data(), 0);

		glVertexArrayVertexBuffer(m_VertexArray, 0, m_VertexBuffer, 0, sizeof(decltype(mesh.Vertices())::value_type));
		glVertexArrayElementBuffer(m_VertexArray, m_IndexBuffer);
	}


	auto GlMesh::DrawWithMaterial(ShaderProgram& shader, GLuint nextFreeTextureUnit, GLsizei const instanceCount) const -> void
	{
		shader.SetUniform("u_Material.diffuseColor", static_cast<Vector3>(m_Material->DiffuseColor));
		shader.SetUniform("u_Material.specularColor", static_cast<Vector3>(m_Material->SpecularColor));
		shader.SetUniform("u_Material.gloss", m_Material->Gloss);
		shader.SetUniform("u_Material.opacity", m_Material->Opacity);

		if (m_Material->DiffuseMap != nullptr)
		{
			shader.SetUniform("u_Material.hasDiffuseMap", true);
			shader.SetUniform("u_Material.diffuseMap", static_cast<GLint>(nextFreeTextureUnit)); // cast to GLint because only glUniform1i[v] may be used to set sampler uniforms (wtf?)
			glBindTextureUnit(nextFreeTextureUnit, m_Material->DiffuseMap->TextureName());
			++nextFreeTextureUnit;
		}
		else
		{
			shader.SetUniform("u_Material.hasDiffuseMap", false);
		}

		if (m_Material->SpecularMap != nullptr)
		{
			shader.SetUniform("u_Material.hasSpecularMap", true);
			shader.SetUniform("u_Material.specularMap", static_cast<GLint>(nextFreeTextureUnit)); // cast to GLint because only glUniform1i[v] may be used to set sampler uniforms (wtf?)
			glBindTextureUnit(nextFreeTextureUnit, m_Material->SpecularMap->TextureName());
			++nextFreeTextureUnit;
		}
		else
		{
			shader.SetUniform("u_Material.hasSpecularMap", false);
		}

		if (m_Material->OpacityMap)
		{
			shader.SetUniform("u_Material.hasOpacityMap", true);
			shader.SetUniform("u_Material.opacityMap", static_cast<GLint>(nextFreeTextureUnit)); // cast to GLint because only glUniform1i[v] may be used to set sampler uniforms (wtf?)
			glBindTextureUnit(nextFreeTextureUnit, m_Material->OpacityMap->TextureName());
		}
		else
		{
			shader.SetUniform("u_Material.hasOpacityMap", false);
		}

		DrawWithoutMaterial(instanceCount);
	}


	auto GlMesh::DrawWithoutMaterial(GLsizei const instanceCount) const -> void
	{
		if (m_Material->TwoSided)
		{
			glEnable(GL_CULL_FACE);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}

		glBindVertexArray(m_VertexArray);
		glDrawElementsInstanced(GL_TRIANGLES, m_NumIndices, GL_UNSIGNED_INT, nullptr, instanceCount);
	}


	auto GlMesh::Material() const noexcept -> std::shared_ptr<leopph::Material const> const&
	{
		return m_Material;
	}
}
