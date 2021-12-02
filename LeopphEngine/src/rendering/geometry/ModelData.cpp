#include "ModelData.hpp"


namespace leopph::impl
{
	ModelData::ModelData(std::vector<impl::MeshData>& meshData) :
		MeshData{meshData}
	{}

	ModelData::~ModelData() = default;
}