#include "Poelo.hpp"

#include "DataManager.hpp"


namespace leopph::internal
{
	auto Poelo::TakeOwnership(Poelo* poelo) -> void
	{
		DataManager::Instance().Store(std::unique_ptr<Poelo>{poelo});
	}


	auto Poelo::Destroy(const Poelo* poelo) -> void
	{
		DataManager::Instance().Destroy(poelo);
	}
}
