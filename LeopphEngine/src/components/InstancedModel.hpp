#pragma once

#include "Component.hpp"
#include "../api/LeopphApi.hpp"

#include <filesystem>


namespace leopph
{
	namespace impl
	{
		class InstancedModelImpl;
	}


	// The Model class represents a drawable object that can be attached to Entites.
	class InstancedModel final : public Component
	{
	public:
		// Load a Model from a file on disk.
		LEOPPHAPI explicit InstancedModel(leopph::Entity& owner, std::filesystem::path path);
		LEOPPHAPI InstancedModel(const InstancedModel& other) = delete;
		LEOPPHAPI InstancedModel(InstancedModel&& other) = delete;

		LEOPPHAPI ~InstancedModel() override;

		LEOPPHAPI InstancedModel& operator=(const InstancedModel& other) = delete;
		LEOPPHAPI InstancedModel& operator=(InstancedModel&& other) = delete;

		// File path of the loaded InstancedModel.
		const std::filesystem::path Path;

		/* Get whether the Model occludes light from other Models.
		 * This only works if the Light used also has this property set to true.
		 * This value is false by default. */
		LEOPPHAPI bool CastsShadow() const;

		/* Set whether the Model occludes light from other Models.
		 * This only works if the Light used also has this property set to true.
		 * This value is false by default. */
		LEOPPHAPI void CastsShadow(bool value) const;


	private:
		impl::InstancedModelImpl* m_Impl;
	};
}