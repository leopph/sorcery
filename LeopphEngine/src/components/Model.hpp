#pragma once

#include "../api/leopphapi.h"
#include "../data/managed/ResourceHandle.hpp"
#include "../rendering/geometry/ModelResource.hpp"
#include "Component.hpp"

#include <filesystem>


namespace leopph
{
	class Entity;

	/*-----------------------------------------------------------------------------
	The Model class represents a drawable object that can be attached to Entites.
	The way it is rendered depends on the owning Entity's current state in space.
	See "Entity.hpp" for more information.
	-----------------------------------------------------------------------------*/
	class Model final : public Component, public impl::ResourceHandle<impl::ModelResource>
	{
	public:
		LEOPPHAPI explicit Model(Entity& owner, const std::filesystem::path& path);

		LEOPPHAPI Model(const Model& other) = delete;
		LEOPPHAPI Model(Model&& other) = delete;

		LEOPPHAPI ~Model() override;

		LEOPPHAPI Model& operator=(const Model& other) = delete;
		LEOPPHAPI Model& operator=(Model&& other) = delete;

		LEOPPHAPI const std::filesystem::path& Path() const;

		LEOPPHAPI bool CastsShadow() const;
		LEOPPHAPI void CastsShadow(bool value);
	};
}