#pragma once

#include "ModelData.hpp"
#include "../shaders/ShaderProgram.hpp"

#include <cstddef>


namespace leopph::impl
{
	class Renderable
	{
	public:
		Renderable(ModelData& modelData);
		Renderable(const Renderable& other) = delete;
		Renderable(Renderable&& other) = delete;

		virtual ~Renderable() = default;

		Renderable& operator=(const Renderable& other) = delete;
		Renderable& operator=(Renderable&& other) = delete;

		virtual void DrawShaded(ShaderProgram& shader, std::size_t nextFreeTextureUnit) const = 0;
		virtual void DrawDepth() const = 0;

		// Update the Renderable by reloading its data from its ModelData source.
		virtual void Update() = 0;

		[[nodiscard]]
		bool CastsShadow() const;
		void CastsShadow(bool value);

		ModelData& ModelDataSrc;


	private:
		bool m_CastsShadow;
	};
}