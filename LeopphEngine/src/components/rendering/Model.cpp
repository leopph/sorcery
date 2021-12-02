#include "Model.hpp"


namespace leopph
{
	Model::Model(leopph::Entity& owner, std::filesystem::path path) :
		impl::RenderComponent{owner}, Path{path}, m_Impl{/*create new ModelImpl*/}
	{

	}

	Model::~Model()
	{
		// delete m_Impl
	}

	bool Model::CastsShadow() const
	{
		//return m_Impl->CastsShadow();
		return false;
	}

	void Model::CastsShadow(bool value)
	{
		//m_Impl->CastsShadow(value);
	}
}