#include "StaticMaterial.hpp"

#include "Context.hpp"
#include "GlCore.hpp"
#include "Renderer.hpp"
#include "Util.hpp"


namespace leopph
{
	void StaticMaterial::bind_and_set_renderstate(u32 const index) const
	{
		if (mCullBackFace)
		{
			glEnable(GL_CULL_FACE);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}

		glBindBufferRange(GL_UNIFORM_BUFFER, index, mBuffer.get_internal_handle(), 0, clamp_cast<GLsizeiptr>(mBuffer.get_size()));
	}



	StaticMaterial::StaticMaterial(StaticMaterialData const& data, std::span<std::shared_ptr<Texture2D const> const> const textures) :
		mBuffer
		{
			// Buffer layout: 
			16 + // vec4 for diffuseColor
			16 + // vec4 for specular color + gloss
			8 +  // diffuseMap handle
			8 +  // specularMap handle
			4 +  // diffuseMap flag
			4 +  // specularMap flag
			4    // alpha threshold
		},
		mCullBackFace{data.cullBackFace}
	{
		auto* const bufferData = static_cast<u8*>(mBuffer.get_ptr());

		*reinterpret_cast<Vector4*>(bufferData) = Vector4{data.diffuseColor.red, data.diffuseColor.green, data.diffuseColor.blue, data.diffuseColor.alpha} / 255.f;
		*reinterpret_cast<Vector4*>(bufferData + 16) = Vector4{Vector3{data.specularColor.red, data.specularColor.green, data.specularColor.red} / 255.f, data.gloss};
		*reinterpret_cast<f32*>(bufferData + 56) = data.alphaThreshold;

		if (data.diffuseMapIndex)
		{
			mTextures.emplace_back(textures[*data.diffuseMapIndex]);
			*reinterpret_cast<u64*>(bufferData + 32) = mTextures.back()->get_handle();
			*reinterpret_cast<i32*>(bufferData + 48) = true;
		}
		else
		{
			*reinterpret_cast<u64*>(bufferData + 32) = 0;
			*reinterpret_cast<i32*>(bufferData + 48) = false;
		}

		if (data.specularMapIndex)
		{
			mTextures.emplace_back(textures[*data.specularMapIndex]);
			*reinterpret_cast<u64*>(bufferData + 40) = mTextures.back()->get_handle();
			*reinterpret_cast<i32*>(bufferData + 52) = true;
		}
		else
		{
			*reinterpret_cast<u64*>(bufferData + 40) = 0;
			*reinterpret_cast<i32*>(bufferData + 52) = false;
		}

		internal::get_renderer()->register_static_material(weak_from_this());
	}



	StaticMaterial::~StaticMaterial()
	{
		internal::get_renderer()->unregister_static_material(weak_from_this());
	}
}
