#pragma once

#include "../../rendering/geometry/ModelData.hpp"
#include "../../rendering/geometry/InstancedModelImpl.hpp"

#include <functional>
#include <cstddef>


namespace leopph::impl
{
	class InstancedModelImplHash
	{
	public:
		using is_transparent = void;

		std::size_t operator()(const InstancedModelImpl& model) const;
		std::size_t operator()(const ModelData& modelData) const;


	private:
		std::hash<const ModelData*> m_Hash;
	};
}