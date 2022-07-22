/*#include "Material.hpp"

#include <utility>



namespace leopph
{
	Material::Property::Property(std::string name, bool const b) :
		mName{std::move(name)},
		mType{Type::Bool},
		mData{new bool{b}}
	{ }



	Material::Property::Property(std::string name, i32 const i) :
		mName{std::move(name)},
		mType{Type::Int},
		mData{new i32{i}}
	{ }



	Material::Property::Property(std::string name, u32 const u) :
		mName{std::move(name)},
		mType{Type::Uint},
		mData{new u32{u}}
	{ }



	Material::Property::Property(std::string name, f32 f) :
		mName{std::move(name)},
		mType{Type::Float},
		mData{new f32{f}}
	{}



	Material::Property::Property(std::string name, Vector2 const& v) :
		mName{std::move(name)},
		mType{Type::Vector2},
		mData{new Vector2{v}}
	{}



	Material::Property::Property(std::string name, Vector3 const& v) :
		mName{std::move(name)},
		mType{Type::Vector3},
		mData{new Vector3{v}}
	{}



	Material::Property::Property(std::string name, Vector4 const& v) :
		mName{std::move(name)},
		mType{Type::Vector4},
		mData{new Vector4{v}}
	{}



	Material::Property::Property(std::string name, Matrix3 const& m) :
		mName{std::move(name)},
		mType{Type::Matrix3},
		mData{new Matrix3{m}}
	{}



	Material::Property::Property(std::string name, Matrix4 const& m) :
		mName{std::move(name)},
		mType{Type::Matrix4},
		mData{new Matrix4{m}}
	{}



	Material::Property::Property(std::string name, std::shared_ptr<GlTexture> t)
	{}



	void Material::Property::set_bool_value(bool b)
	{}



	void Material::Property::set_i32_value(i32 i)
	{}



	void Material::Property::set_u32_value(u32 u)
	{}



	void Material::Property::set_f32_value(f32 f)
	{}



	void Material::Property::set_vector2_value(Vector2 const& v)
	{}



	void Material::Property::set_vector3_value(Vector3 const& v)
	{}



	void Material::Property::set_vector4_value(Vector4 const& v)
	{}



	void Material::Property::set_matrix3_value(Matrix3 const& m)
	{}



	void Material::Property::set_matrix4_value(Matrix4 const& m)
	{}



	void Material::Property::set_texture_value(std::shared_ptr<GlTexture> t)
	{}



	std::string_view Material::Property::get_name() const
	{
		return mName;
	}



	Material::Property::Type Material::Property::get_type() const
	{
		return mType;
	}



	/*std::optional<bool> Material::Property::get_bool_value() const
	{
		if (mType != Type::Bool)
		{
			return std::nullopt;
		}
		return *static_cast<bool*>(mData);
	}



	std::optional<i32> Material::Property::get_i32_value() const
	{
		if (mType != Type::Int)
		{
			return std::nullopt;
		}
		return *static_cast<i32*>(mData);
	}



	std::optional<u32> Material::Property::get_u32_value() const
	{
		if (mType != Type::Uint)
		{
			return std::nullopt;
		}
		return *static_cast<u32*>(mData);
	}



	std::optional<f32> Material::Property::get_f32_value() const
	{
		if (mType != Type::Float)
		{
			return std::nullopt;
		}
		return *static_cast<f32*>(mData);
	}



	std::optional<Vector2> Material::Property::get_vector2_value() const
	{
		if (mType != Type::Vector2)
		{
			return std::nullopt;
		}

		return *static_cast<Vector2*>(mData);
	}



	std::optional<Vector3> Material::Property::get_vector3_value() const
	{
		if (mType != Type::Vector3)
		{
			return std::nullopt;
		}
		return *static_cast<Vector3*>(mData);
	}



	std::optional<Vector4> Material::Property::get_vector4_value() const
	{
		if (mType != Type::Vector4)
		{
			return std::nullopt;
		}
		return *static_cast<Vector4*>(mData);
	}



	std::optional<Matrix3> Material::Property::get_matrix3_value() const
	{
		if (mType != Type::Matrix3)
		{
			return std::nullopt;
		}
		return *static_cast<Matrix3*>(mData);
	}



	std::optional<Matrix4> Material::Property::get_matrix4_value() const
	{
		if (mType != Type::Matrix4)
		{
			return std::nullopt;
		}
		return *static_cast<Matrix4*>(mData);
	}



	std::optional<std::shared_ptr<GlTexture>> Material::Property::get_texture_value() const
	{
		if (mType == Type::Texture)
		{
			return std::nullopt;
		}
		return *static_cast<std::shared_ptr<GlTexture>*>(mData);
	}
}
*/