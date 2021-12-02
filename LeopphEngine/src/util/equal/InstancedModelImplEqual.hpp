#pragma once

#include "../../rendering/geometry/ModelData.hpp"
#include "../../rendering/geometry/InstancedModelImpl.hpp"


namespace leopph::impl
{
	class InstancedModelImplEqual
	{
	public:
		using is_transparent = void;

		bool operator()(const InstancedModelImpl& left, const InstancedModelImpl& right) const;
		bool operator()(const InstancedModelImpl& left, const ModelData& right) const;
		bool operator()(const ModelData& left, const InstancedModelImpl& right) const;
	};
}