#include "Poelo.hpp"

#include "DataManager.hpp"


namespace leopph
{
	namespace internal
	{
		Poelo::Poelo()
		{
			DataManager::Instance().Store(std::unique_ptr<Poelo>{this});
		}
	}


	auto Destroy(const internal::Poelo* poelo) -> void
	{
		internal::DataManager::Instance().Destroy(poelo);
	}
}
