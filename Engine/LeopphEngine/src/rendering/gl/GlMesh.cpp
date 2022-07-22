#include "rendering/gl/GlMesh.hpp"

#include "Matrix.hpp"
#include "Vector.hpp"


namespace leopph::internal
{
	GlMesh::GlMesh(Mesh const& mesh, GLuint const instanceBuffer) :
		mNumIndices{static_cast<GLsizei>(mesh.Indices().size())},
		mMaterial{mesh.Material()}
	{
		// Add the instance buffer to the vertex array.
		glVertexArrayVertexBuffer(mVao, 1, instanceBuffer, 0, sizeof(std::pair<Matrix4, Matrix4>));

		// Specify attributes in Vertex Array

		// Position
		glVertexArrayAttribFormat(mVao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(decltype(mesh.Vertices())::value_type, Position));
		glVertexArrayAttribBinding(mVao, 0, 0);
		glEnableVertexArrayAttrib(mVao, 0);

		// Normal
		glVertexArrayAttribFormat(mVao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(decltype(mesh.Vertices())::value_type, Normal));
		glVertexArrayAttribBinding(mVao, 1, 0);
		glEnableVertexArrayAttrib(mVao, 1);

		// Texture coordinates
		glVertexArrayAttribFormat(mVao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(decltype(mesh.Vertices())::value_type, TexCoord));
		glVertexArrayAttribBinding(mVao, 2, 0);
		glEnableVertexArrayAttrib(mVao, 2);

		// Model matrix 1st row
		glVertexArrayAttribFormat(mVao, 3, 4, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(mVao, 3, 1);
		glEnableVertexArrayAttrib(mVao, 3);

		// Model matrix 2nd row
		glVertexArrayAttribFormat(mVao, 4, 4, GL_FLOAT, GL_FALSE, sizeof(Vector4));
		glVertexArrayAttribBinding(mVao, 4, 1);
		glEnableVertexArrayAttrib(mVao, 4);

		// Model matrix 3rd row
		glVertexArrayAttribFormat(mVao, 5, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(Vector4));
		glVertexArrayAttribBinding(mVao, 5, 1);
		glEnableVertexArrayAttrib(mVao, 5);

		// Model matrix 4th row
		glVertexArrayAttribFormat(mVao, 6, 4, GL_FLOAT, GL_FALSE, 3 * sizeof(Vector4));
		glVertexArrayAttribBinding(mVao, 6, 1);
		glEnableVertexArrayAttrib(mVao, 6);

		// Normal matrix 1st row
		glVertexArrayAttribFormat(mVao, 7, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(Vector4));
		glVertexArrayAttribBinding(mVao, 7, 1);
		glEnableVertexArrayAttrib(mVao, 7);

		// Normal matrix 2nd row
		glVertexArrayAttribFormat(mVao, 8, 4, GL_FLOAT, GL_FALSE, 5 * sizeof(Vector4));
		glVertexArrayAttribBinding(mVao, 8, 1);
		glEnableVertexArrayAttrib(mVao, 8);

		// Normal matrix 3rd row
		glVertexArrayAttribFormat(mVao, 9, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(Vector4));
		glVertexArrayAttribBinding(mVao, 9, 1);
		glEnableVertexArrayAttrib(mVao, 9);

		// Normal matrix 4th row
		glVertexArrayAttribFormat(mVao, 10, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(Vector4));
		glVertexArrayAttribBinding(mVao, 10, 1);
		glEnableVertexArrayAttrib(mVao, 10);

		// Set the attributes to advance once per instance.
		glVertexArrayBindingDivisor(mVao, 1, 1);

		glNamedBufferStorage(mVbo, mesh.Vertices().size() * sizeof(decltype(mesh.Vertices())::value_type), mesh.Vertices().data(), 0);
		glNamedBufferStorage(mIbo, mesh.Indices().size() * sizeof(decltype(mesh.Indices())::value_type), mesh.Indices().data(), 0);

		glVertexArrayVertexBuffer(mVao, 0, mVbo, 0, sizeof(decltype(mesh.Vertices())::value_type));
		glVertexArrayElementBuffer(mVao, mIbo);
	}



	void GlMesh::draw_with_material(gsl::not_null<ShaderProgram const*> const shader, GLuint nextFreeTextureUnit, GLsizei const instanceCount) const
	{
		shader->set_uniform("u_Material.diffuseColor", static_cast<Vector3>(mMaterial->DiffuseColor));
		shader->set_uniform("u_Material.specularColor", static_cast<Vector3>(mMaterial->SpecularColor));
		shader->set_uniform("u_Material.gloss", mMaterial->Gloss);
		shader->set_uniform("u_Material.opacity", mMaterial->Opacity);

		if (mMaterial->DiffuseMap != nullptr)
		{
			shader->set_uniform("u_Material.hasDiffuseMap", true);
			shader->set_uniform("u_Material.diffuseMap", static_cast<GLint>(nextFreeTextureUnit)); // cast to GLint because only glUniform1i[v] may be used to set sampler uniforms (wtf?)
			glBindTextureUnit(nextFreeTextureUnit, mMaterial->DiffuseMap->TextureName());
			++nextFreeTextureUnit;
		}
		else
		{
			shader->set_uniform("u_Material.hasDiffuseMap", false);
		}

		if (mMaterial->SpecularMap != nullptr)
		{
			shader->set_uniform("u_Material.hasSpecularMap", true);
			shader->set_uniform("u_Material.specularMap", static_cast<GLint>(nextFreeTextureUnit)); // cast to GLint because only glUniform1i[v] may be used to set sampler uniforms (wtf?)
			glBindTextureUnit(nextFreeTextureUnit, mMaterial->SpecularMap->TextureName());
			++nextFreeTextureUnit;
		}
		else
		{
			shader->set_uniform("u_Material.hasSpecularMap", false);
		}

		if (mMaterial->OpacityMap)
		{
			shader->set_uniform("u_Material.hasOpacityMap", true);
			shader->set_uniform("u_Material.opacityMap", static_cast<GLint>(nextFreeTextureUnit)); // cast to GLint because only glUniform1i[v] may be used to set sampler uniforms (wtf?)
			glBindTextureUnit(nextFreeTextureUnit, mMaterial->OpacityMap->TextureName());
		}
		else
		{
			shader->set_uniform("u_Material.hasOpacityMap", false);
		}

		draw_without_material(instanceCount);
	}



	void GlMesh::draw_without_material(GLsizei const instanceCount) const
	{
		if (mMaterial->TwoSided)
		{
			glEnable(GL_CULL_FACE);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}

		glBindVertexArray(mVao);
		glDrawElementsInstanced(GL_TRIANGLES, mNumIndices, GL_UNSIGNED_INT, nullptr, instanceCount);
	}



	std::shared_ptr<Material const> const& GlMesh::get_material() const noexcept
	{
		return mMaterial;
	}
}
