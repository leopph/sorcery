#pragma once

#include "../../api/leopphapi.h"
#include "../Component.hpp"

#include <filesystem>

namespace leopph
{
	namespace impl
	{
		class AssimpModelImpl;
	}

	class Object;

	/*-----------------------------------------------------------------------------
	The Model class represents a drawable entity that can be attached to Objects.
	The way it is rendered depends on the owning Object's current state in space.
	See "object.h" for more information.
	-----------------------------------------------------------------------------*/
	class Model final : public Component
	{
	public:
		LEOPPHAPI explicit Model(Object& owner, const std::filesystem::path& path);
		LEOPPHAPI ~Model() override;

		Model(const Model&) = delete;
		Model(Model&&) = delete;
		void operator=(const Model&) = delete;
		void operator=(Model&&) = delete;

		bool operator==(const Model& other) const;

		LEOPPHAPI const std::filesystem::path& Path() const;

	private:
		const impl::AssimpModelImpl* m_Ref;
	};
}