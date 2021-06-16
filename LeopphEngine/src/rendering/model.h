#pragma once

#include <filesystem>
#include "../api/leopphapi.h"
#include "shader.h"

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

	class Model
	{
	public:
		LEOPPHAPI Model(std::filesystem::path path);
		LEOPPHAPI Model(const Model& other);
		LEOPPHAPI Model(Model&& other) noexcept;

		LEOPPHAPI ~Model();
		
		LEOPPHAPI Model& operator=(const Model& other);
		LEOPPHAPI Model& operator=(Model&& other) noexcept;

		LEOPPHAPI bool operator==(const Model& other) const;

		/* TODO this should not be accessible to user */
		LEOPPHAPI void Draw(const impl::Shader& shader) const;

		LEOPPHAPI const std::filesystem::path& Path() const;

	private:
		impl::AssimpModelImpl* m_Pointer;
	};
}