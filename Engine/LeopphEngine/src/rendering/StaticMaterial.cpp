#include "StaticMaterial.hpp"

#include "GlCore.hpp"
#include "Util.hpp"
#include "Vector.hpp"


namespace leopph
{
	StaticMaterial::StaticMaterial(StaticMaterialData const& data) :
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

		if (data.diffuseMap)
		{
			mTextures.emplace_back(new Texture2D{*data.diffuseMap});
			*reinterpret_cast<u64*>(bufferData + 32) = mTextures.back()->internal_handle();
			*reinterpret_cast<i32*>(bufferData + 48) = true;
		}
		else
		{
			*reinterpret_cast<u64*>(bufferData + 32) = 0;
			*reinterpret_cast<i32*>(bufferData + 48) = false;
		}

		if (data.specularMap)
		{
			mTextures.emplace_back(new Texture2D{*data.specularMap});
			*reinterpret_cast<u64*>(bufferData + 40) = mTextures.back()->internal_handle();
			*reinterpret_cast<i32*>(bufferData + 52) = true;
		}
		else
		{
			*reinterpret_cast<u64*>(bufferData + 40) = 0;
			*reinterpret_cast<i32*>(bufferData + 52) = false;
		}
	}



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
}
