#include "model.h"
#include "assimpmodel.h"

namespace leopph
{
	Model::Model(const std::filesystem::path& path) :
		m_Pointer{ new impl::AssimpModelImpl{path} }
	{}

	Model::Model(const Model& other) :
		m_Pointer{ new impl::AssimpModelImpl{*other.m_Pointer} }
	{}

	Model::Model(Model&& other) noexcept :
		m_Pointer{ other.m_Pointer }
	{
		other.m_Pointer = nullptr;
	}


	Model::~Model()
	{
		delete m_Pointer;
	}

	
	Model& Model::operator=(const Model& other)
	{
		delete m_Pointer;
		m_Pointer = new impl::AssimpModelImpl{ *other.m_Pointer };
		return *this;
	}

	Model& Model::operator=(Model&& other) noexcept
	{
		delete m_Pointer;
		m_Pointer = other.m_Pointer;
		other.m_Pointer = nullptr;
		return *this;
	}


	bool Model::operator==(const Model& other) const
	{
		return *m_Pointer == *other.m_Pointer;
	}


	void Model::Draw(const implementation::Shader& shader) const
	{
		m_Pointer->Draw(shader);
	}
}