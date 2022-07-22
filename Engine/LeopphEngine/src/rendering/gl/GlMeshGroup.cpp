#include "rendering/gl/GlMeshGroup.hpp"

#include "DataManager.hpp"

#include <algorithm>
#include <utility>


namespace leopph::internal
{
	GlMeshGroup::GlMeshGroup(leopph::MeshGroup meshGroup) :
		mMeshGroup{std::move(meshGroup)}
	{
		glNamedBufferData(mInstanceBuffer, 2 * sizeof(Matrix4), nullptr, GL_DYNAMIC_DRAW);
		update_mesh_data();
	}



	void GlMeshGroup::draw_with_material(gsl::not_null<ShaderProgram const*> shader, GLuint const nextFreeTextureUnit, bool const transparent) const
	{
		for (auto const& mesh : transparent ? mGuaranteedTransparentMeshes : mOpaqueMeshes)
		{
			mesh->draw_with_material(shader, nextFreeTextureUnit, mInstanceBufferSize);
		}

		for (auto const& mesh : mPotentiallyTransparentMeshes)
		{
			mesh->draw_with_material(shader, nextFreeTextureUnit, mInstanceBufferSize);
		}
	}



	void GlMeshGroup::draw_without_material(bool const transparent) const
	{
		for (auto const& mesh : transparent ? mGuaranteedTransparentMeshes : mOpaqueMeshes)
		{
			mesh->draw_without_material(mInstanceBufferSize);
		}

		for (auto const& mesh : mPotentiallyTransparentMeshes)
		{
			mesh->draw_without_material(mInstanceBufferSize);
		}
	}



	void GlMeshGroup::set_instance_data(std::span<std::pair<Matrix4, Matrix4> const> const instMats)
	{
		mInstanceBufferSize = static_cast<GLsizei>(instMats.size());

		// If there are more instances than we what we have space for we reallocate the buffer
		if (mInstanceBufferSize > mInstanceBufferCapacity)
		{
			do
			{
				mInstanceBufferCapacity *= 2;
			}
			while (mInstanceBufferSize > mInstanceBufferCapacity);

			glNamedBufferData(mInstanceBuffer, static_cast<GLsizei>(mInstanceBufferCapacity * sizeof(std::remove_reference_t<decltype(instMats)>::value_type)), nullptr, GL_DYNAMIC_DRAW);
		}

		glNamedBufferSubData(mInstanceBuffer, 0, static_cast<GLsizei>(mInstanceBufferSize * sizeof(std::remove_reference_t<decltype(instMats)>::value_type)), instMats.data());
	}



	leopph::MeshGroup const& GlMeshGroup::get_mesh_group() const
	{
		return mMeshGroup;
	}



	void GlMeshGroup::set_mesh_group(leopph::MeshGroup meshGroup)
	{
		mMeshGroup = std::move(meshGroup);
		update_mesh_data();
	}



	void GlMeshGroup::update_mesh_data()
	{
		mOpaqueMeshes.clear();
		mPotentiallyTransparentMeshes.clear();
		mGuaranteedTransparentMeshes.clear();

		for (auto const& mesh : mMeshGroup.Meshes())
		{
			(is_guaranteed_transparent(mesh.Material()) ? mGuaranteedTransparentMeshes : mOpaqueMeshes).emplace_back(std::make_unique<GlMesh>(mesh, mInstanceBuffer));
		}
	}



	void GlMeshGroup::sort_meshes()
	{
		std::ranges::for_each(mOpaqueMeshes, [this](std::unique_ptr<GlMesh>& glMesh)
		{
			if (is_guaranteed_transparent(glMesh->get_material()))
			{
				mGuaranteedTransparentMeshes.emplace_back(std::move(glMesh));
			}
			else if (is_potentially_transparent(glMesh->get_material()))
			{
				mPotentiallyTransparentMeshes.emplace_back(std::move(glMesh));
			}
		});

		std::erase_if(mOpaqueMeshes, [](std::unique_ptr<GlMesh> const& glMesh)
		{
			return !glMesh;
		});

		std::ranges::for_each(mPotentiallyTransparentMeshes, [this](std::unique_ptr<GlMesh>& glMesh)
		{
			if (!is_potentially_transparent(glMesh->get_material()))
			{
				(is_guaranteed_transparent(glMesh->get_material()) ? mGuaranteedTransparentMeshes : mOpaqueMeshes).emplace_back(std::move(glMesh));
			}
		});

		std::erase_if(mPotentiallyTransparentMeshes, [](std::unique_ptr<GlMesh> const& glMesh)
		{
			return !glMesh;
		});

		std::ranges::for_each(mGuaranteedTransparentMeshes, [this](std::unique_ptr<GlMesh>& glMesh)
		{
			if (!is_guaranteed_transparent(glMesh->get_material()))
			{
				(is_potentially_transparent(glMesh->get_material()) ? mPotentiallyTransparentMeshes : mOpaqueMeshes).emplace_back(std::move(glMesh));
			}
		});

		std::erase_if(mGuaranteedTransparentMeshes, [](std::unique_ptr<GlMesh> const& glMesh)
		{
			return !glMesh;
		});
	}



	bool GlMeshGroup::is_guaranteed_transparent(std::shared_ptr<Material const> const& mat)
	{
		return mat->Opacity < 1.f;
	}



	bool GlMeshGroup::is_potentially_transparent(std::shared_ptr<Material const> const& mat)
	{
		return mat->OpacityMap != nullptr;
	}
}
