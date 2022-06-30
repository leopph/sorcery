#include "Poelo.hpp"

#include "DataManager.hpp"
#include "InternalContext.hpp"


namespace leopph
{
	namespace internal
	{
		Poelo::Poelo()
		{
			GetDataManager()->Store(std::unique_ptr<Poelo>{this});
		}
	}


	auto Destroy(internal::Poelo const* poelo) -> void
	{
		internal::GetDataManager()->Destroy(poelo);
	}
}
