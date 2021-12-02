#pragma once

#include "RenderComponent.hpp"
#include "../../api/LeopphApi.hpp"

#include <filesystem>


namespace leopph
{
	namespace impl
	{
		class InstancedModelImpl;
	}


	// The InstancedModel class represents drawable objects that share internal resources to speed up rendering.
	class InstancedModel final : public impl::RenderComponent
	{
	public:
		// Load a Model from a file on disk.
		LEOPPHAPI InstancedModel(leopph::Entity& owner, std::filesystem::path path);
		InstancedModel(const InstancedModel& other) = delete;
		InstancedModel(InstancedModel&& other) = delete;

		LEOPPHAPI ~InstancedModel() override;

		InstancedModel& operator=(const InstancedModel& other) = delete;
		InstancedModel& operator=(InstancedModel&& other) = delete;

		// File path of the loaded InstancedModel.
		const std::filesystem::path Path;

		/* Get whether the InstancedModel occludes light from other objects.
		 * This only works if the Light used also has this property set to true.
		 * This value is false by default. */
		[[nodiscard]]
		LEOPPHAPI bool CastsShadow() const override;

		/* Set whether the InstancedModel occludes light from other objects.
		 * This only works if the Light used also has this property set to true.
		 * This value is false by default. */
		LEOPPHAPI void CastsShadow(bool value) override;


	private:
		impl::InstancedModelImpl* m_Impl;
	};
}