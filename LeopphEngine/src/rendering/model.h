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


	// TODO document
	// TODO public path or other ID needed
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

		LEOPPHAPI void Draw(const impl::Shader& shader) const;

		LEOPPHAPI const std::filesystem::path& Path() const;

	private:
		impl::AssimpModelImpl* m_Pointer;
	};
}