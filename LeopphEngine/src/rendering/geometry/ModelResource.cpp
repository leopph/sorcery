#include "ModelResource.hpp"

#include "AssimpModel.hpp"
#include "AssimpModel.hpp"
#include "AssimpModel.hpp"
#include "AssimpModel.hpp"
#include "../../data/DataManager.hpp"
#include "../../util/logger.h"
#include "AssimpModel.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/matrix4x4.h>

#include <stdexcept>
#include <string>
#include <queue>
#include <utility>


namespace leopph::impl
{
	ModelResource::ModelResource(const std::filesystem::path& path) :
		UniqueResource{ path }, m_AssimpModel { new AssimpModel{ path } }, m_CastsShadow{false}
	{}

	
	ModelResource::~ModelResource()
	{
		delete m_AssimpModel;
	}


	void ModelResource::DrawShaded(::leopph::impl::ShaderProgram& shader, const std::vector<std::pair<Matrix4, Matrix4>>& instanceMatrices, std::size_t nextFreeTextureUnit)
	{
		m_AssimpModel->DrawShaded(shader, instanceMatrices, nextFreeTextureUnit);
	}


	void ModelResource::DrawDepth(const std::vector<std::pair<Matrix4, Matrix4>>& instanceMatrices)
	{
		m_AssimpModel->DrawDepth(instanceMatrices);
	}


	bool ModelResource::CastsShadow() const
	{
		return m_CastsShadow;
	}


	void ModelResource::CastsShadow(const bool value)
	{
		m_CastsShadow = value;
	}

}
