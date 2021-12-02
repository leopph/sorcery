#include "InstancedModelImplHash.hpp"


namespace leopph::impl
{
	std::size_t InstancedModelImplHash::operator()(const InstancedModelImpl& model) const
	{
		return m_Hash(& model.ModelDataSrc);
	}

	std::size_t InstancedModelImplHash::operator()(const ModelData& modelData) const
	{
		return m_Hash(&modelData);
	}
}