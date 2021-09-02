#pragma once

#include "../api/leopphapi.h"
#include "../data/managed/ResourceHandle.hpp"
#include "../rendering/geometry/ModelResource.hpp"
#include "Component.hpp"

#include <filesystem>


namespace leopph
{
	class Object;

	/*-----------------------------------------------------------------------------
	The Model class represents a drawable entity that can be attached to Objects.
	The way it is rendered depends on the owning Object's current state in space.
	See "object.h" for more information.
	-----------------------------------------------------------------------------*/
	class Model final : public Component, public impl::ResourceHandle<impl::ModelResource>
	{
	public:
		LEOPPHAPI explicit Model(Object& owner, const std::filesystem::path& path);

		LEOPPHAPI ~Model();

		LEOPPHAPI const std::filesystem::path& Path() const;
	};
}