#include "PoeloBase.hpp"

#include "DataManager.hpp"


namespace leopph::internal
{
	auto PoeloBase::Store(std::unique_ptr<PoeloBase> poeloBase) -> void
	{
		DataManager::Instance().Store(std::move(poeloBase));
	}


	auto PoeloBase::Destroy(const PoeloBase* poeloBase) -> void
	{
		DataManager::Instance().Destroy(poeloBase);
	}
}
