#pragma once

#include "Component.hpp"
#include "../api/LeopphApi.hpp"

#include <filesystem>


namespace leopph
{
	namespace impl
	{
		class ModelImpl;
	}


	// The Model class represents a drawable object that can be attached to Entites.
	class Model final : public Component
	{
	public:
		// Load a Model from a file on disk.
		LEOPPHAPI explicit Model(leopph::Entity& owner, std::filesystem::path path);
		LEOPPHAPI Model(const Model& other) = delete;
		LEOPPHAPI Model(Model&& other) = delete;

		LEOPPHAPI ~Model() override;

		LEOPPHAPI Model& operator=(const Model& other) = delete;
		LEOPPHAPI Model& operator=(Model&& other) = delete;

		// Get the filepath of the loaded Model.
		LEOPPHAPI const std::filesystem::path& Path() const;

		/* Get whether the Model occludes light from other Models.
		 * This only works if the Light used also has this property set to true.
		 * This value is false by default. */
		LEOPPHAPI bool CastsShadow() const;

		/* Set whether the Model occludes light from other Models.
		 * This only works if the Light used also has this property set to true.
		 * This value is false by default. */
		LEOPPHAPI void CastsShadow(bool value) const;


	private:
		impl::ModelImpl* m_Impl;
	};
}