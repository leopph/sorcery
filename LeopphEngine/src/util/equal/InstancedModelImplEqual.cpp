#include "InstancedModelImplEqual.hpp"


namespace leopph::impl
{
	bool InstancedModelImplEqual::operator()(const InstancedModelImpl& left, const InstancedModelImpl& right) const
	{
		return &left.ModelDataSrc == &right.ModelDataSrc;
	}

	bool InstancedModelImplEqual::operator()(const InstancedModelImpl& left, const ModelData& right) const
	{
		return &left.ModelDataSrc == &right;
	}

	bool InstancedModelImplEqual::operator()(const ModelData& left, const InstancedModelImpl& right) const
	{
		return &left == &right.ModelDataSrc;
	}
}