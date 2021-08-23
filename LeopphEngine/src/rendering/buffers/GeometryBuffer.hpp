#pragma once

#include "Framebuffer.hpp"

#include <array>


namespace leopph::impl
{
	class GeometryBuffer : public Framebuffer
	{
	public:
		GeometryBuffer();

		GeometryBuffer(const GeometryBuffer&) = delete;
		GeometryBuffer(GeometryBuffer&&) = delete;

		GeometryBuffer& operator=(const GeometryBuffer&) = delete;
		GeometryBuffer& operator=(GeometryBuffer&&) = delete;

		~GeometryBuffer();

		void Clear() const;
		void Bind() const;
		void Unbind() const;

		const unsigned& positionTextureName;
		const unsigned& normalTextureName;
		const unsigned& ambientTextureName;
		const unsigned& diffuseTextureName;
		const unsigned& specularTextureName;

	private:
		enum TextureType
		{
			Position, Normal, Ambient, Diffuse, Specular
		};

		std::array<unsigned, 5> m_Textures;
	};
}
