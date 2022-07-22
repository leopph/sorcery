#pragma once

#include "Color.hpp"
#include "GlTexture.hpp"
#include "Matrix.hpp"
#include "Types.hpp"
#include "Util.hpp"
#include "Vector.hpp"

#include <algorithm>
#include <concepts>
#include <memory>
#include <string>
#include <string_view>
#include <vector>


namespace leopph
{
	/*class Material
	{
		[[nodiscard]] bool get_bool_value() const;
		[[nodiscard]] i32 get_i32_value() const;
		[[nodiscard]] u32 get_u32_value() const;
		[[nodiscard]] f32 get_f32_value() const;
		[[nodiscard]] Vector2 get_vector2_value() const;
		[[nodiscard]] Vector3 get_vector3_value() const;
		[[nodiscard]] Vector4 get_vector4_value() const;
		[[nodiscard]] Matrix3 get_matrix3_value() const;
		[[nodiscard]] Matrix4 get_matrix4_value() const;
		[[nodiscard]] std::shared_ptr<GlTexture> get_texture_value() const;

		void set_bool_value(bool b);
		void set_i32_value(i32 i);
		void set_u32_value(u32 u);
		void set_f32_value(f32 f);
		void set_vector2_value(Vector2 const& v);
		void set_vector3_value(Vector3 const& v);
		void set_vector4_value(Vector4 const& v);
		void set_matrix3_value(Matrix3 const& m);
		void set_matrix4_value(Matrix4 const& m);
		void set_texture_value(std::shared_ptr<GlTexture> t);

		
		class Property
		{
			public:
				enum class Type : u8
				{
					Bool = 0,
					Int = 1,
					Uint = 2,
					Float = 3,
					Vector2 = 4,
					Vector3 = 5,
					Vector4 = 6,
					Matrix3 = 7,
					Matrix4 = 8,
					Texture = 9,
					Unknown
				};


				[[nodiscard]] std::string_view get_name() const;
				[[nodiscard]] Type get_type() const;


			private:
				// ReSharper disable All
				template<Type PropertyType>
				using get_value_type =
				std::conditional_t<PropertyType == Type::Bool, bool,
				                   std::conditional_t<PropertyType == Type::Int, i32,
				                                      std::conditional_t<PropertyType == Type::Uint, u32,
				                                                         std::conditional_t<PropertyType == Type::Float, f32,
				                                                                            std::conditional_t<PropertyType == Type::Vector2, Vector2,
				                                                                                               std::conditional_t<PropertyType == Type::Vector3, Vector3,
				                                                                                                                  std::conditional_t<PropertyType == Type::Vector4, Vector4,
				                                                                                                                                     std::conditional_t<PropertyType == Type::Matrix3, Matrix3,
				                                                                                                                                                        std::conditional_t<PropertyType == Type::Matrix4, Matrix4,
				                                                                                                                                                                           std::conditional_t<PropertyType == Type::Texture, std::shared_ptr<GlTexture>, u8>>>>>>>>>>;

				//template<class T>
				//Type get_enum_type()

				// ReSharper restore All



				std::string mName;
				Type mType;
				void* mData;
		};



		private:
			std::vector<Property> mProperties;
	};*/



	struct Material
	{
		Color DiffuseColor{250, 255, 255};
		Color SpecularColor{0, 0, 0};

		std::shared_ptr<GlTexture> DiffuseMap;
		std::shared_ptr<GlTexture> SpecularMap;
		std::shared_ptr<GlTexture> OpacityMap;

		float Gloss{1};
		float Opacity{1};

		bool TwoSided{false};
	};
}
