#pragma once

#include "../api/leopphapi.h"
#include "../components/component.h"
#include "shader.h"

#include <filesystem>

namespace leopph
{
	namespace impl
	{
		class AssimpModelImpl;
	}


	/*-----------------------------------------------------------------------------
	The Model class represents a renderable entity that can be attached to Objects.
	The way it is rendered depends on the owning Object's current state in space.
	See "object.h" for more information.
	-----------------------------------------------------------------------------*/

	class Model : public Component
	{
	public:
		LEOPPHAPI Model(std::filesystem::path path);
		LEOPPHAPI Model(const Model& other) = delete;
		LEOPPHAPI ~Model();

		LEOPPHAPI void Init() override;
		
		LEOPPHAPI Model& operator=(const Model& other) = delete;
		LEOPPHAPI bool operator==(const Model& other) const;

		LEOPPHAPI const std::filesystem::path& Path() const;

	private:
		const impl::AssimpModelImpl* m_Ref;
	};
}