#pragma once

#include "NonInstancedRenderComponent.hpp"

#include <filesystem>


namespace leopph
{
	// The Model class represents drawable objects whose internal resources are unique.
	class Model : public impl::NonInstancedRenderComponent
	{
		// Load a Model from a file on disk.
		LEOPPHAPI Model(leopph::Entity* entity, std::filesystem::path);

		Model(const Model& other) = delete;
		Model& operator=(const Model& other) = delete;

		Model(Model&& other) = delete;
		Model& operator=(Model&& other) = delete;

		LEOPPHAPI ~Model() override = default;

		// File path of the loaded InstancedModel.
		const std::filesystem::path Path;

		/* Get whether the Model occludes light from other objects.
		 * This only works if the Light used also has this property set to true.
		 * This value is false by default. */
		[[nodiscard]]
		LEOPPHAPI bool CastsShadow() const override;

		/* Set whether the Model occludes light from other objects.
		 * This only works if the Light used also has this property set to true.
		 * This value is false by default. */
		LEOPPHAPI void CastsShadow(bool value) override;
	};
}