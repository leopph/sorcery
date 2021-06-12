#pragma once

#include <filesystem>
#include "leopphapi.h"
#include "shader.h"

namespace leopph
{
	namespace impl
	{
		class AssimpModelImpl;
	}


	class Model
	{
	public:
		LEOPPHAPI Model(const std::filesystem::path& path);
		LEOPPHAPI Model(const Model& other);
		LEOPPHAPI Model(Model&& other) noexcept;

		LEOPPHAPI ~Model();
		
		LEOPPHAPI Model& operator=(const Model& other);
		LEOPPHAPI Model& operator=(Model&& other) noexcept;

		LEOPPHAPI bool operator==(const Model& other) const;

		LEOPPHAPI void Draw(const impl::Shader& shader) const;

	private:
		impl::AssimpModelImpl* m_Pointer;
	};
}