#pragma once

#include "../hierarchy/Object.hpp"
#include "../rendering/assimpmodel.h"

#include <cstddef>
#include <filesystem>

namespace leopph::impl
{
	class ModelReference
	{
	public:
		explicit ModelReference(std::filesystem::path path);

		void AddObject(Object* object);
		void RemoveObject(Object* object);
		[[nodiscard]] std::size_t ReferenceCount() const;
		[[nodiscard]] const std::set<Object*>& Objects() const;

		[[nodiscard]] const AssimpModelImpl& ReferenceModel() const;

	private:
		std::set<Object*> m_Objects;
		AssimpModelImpl m_ReferenceModel;
	};
}