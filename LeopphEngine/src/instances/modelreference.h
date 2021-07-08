#pragma once

#include "../hierarchy/object.h"
#include "../rendering/assimpmodel.h"

#include <filesystem>

namespace leopph::impl
{
	class ModelReference
	{
	public:
		ModelReference(std::filesystem::path path);

		void AddObject(Object* object);
		void RemoveObject(Object* object);
		std::size_t ReferenceCount() const;
		const std::set<Object*> Objects() const;

		const AssimpModelImpl& ReferenceModel() const;

	private:
		std::set<Object*> m_Objects;
		AssimpModelImpl m_ReferenceModel;
	};
}