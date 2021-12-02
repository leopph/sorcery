#pragma once

#include "MeshData.hpp"


namespace leopph::impl
{
	class ModelData
	{
	public:
		ModelData(std::vector<MeshData>& meshData);
		ModelData(const ModelData& other) = delete;
		ModelData(ModelData&& other) = delete;

		virtual ~ModelData() = 0;

		ModelData& operator=(const ModelData& other) = delete;
		ModelData& operator=(ModelData&& other) = delete;

		// A collection of the all the geometric data used in the Model.
		std::vector<MeshData>& MeshData;
	};
}