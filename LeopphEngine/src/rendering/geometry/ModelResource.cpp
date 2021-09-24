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
		UniqueResource{ path }, m_AssimpModel { new AssimpModel{ path } }
	{}

	
	ModelResource::~ModelResource()
	{
		delete m_AssimpModel;
	}


	void ModelResource::DrawShaded(::leopph::impl::ShaderProgram& shader, const std::vector<Matrix4>& modelMatrices, const std::vector<Matrix4>& normalMatrices, std::size_t nextFreeTextureUnit) const
	{
		m_AssimpModel->DrawShaded(shader, modelMatrices, normalMatrices, nextFreeTextureUnit);
	}


	void ModelResource::DrawDepth(const std::vector<Matrix4>& modelMatrices) const
	{
		m_AssimpModel->DrawDepth(modelMatrices);
	}


	void ModelResource::OnEventReceived(const ModelCountChangedEvent& event)
	{
		if (event.Model != this)
		{
			return;
		}

		m_AssimpModel->OnReferringEntitiesChanged(event.Count);
	}
}
