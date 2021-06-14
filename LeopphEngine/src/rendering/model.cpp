#include "model.h"
#include "assimpmodel.h"
#include <utility>

namespace leopph
{
	Model::Model(std::filesystem::path path) :
		m_Pointer{ new impl::AssimpModelImpl{ std::move(path) } }
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


	void Model::Draw(const impl::Shader& shader) const
	{
		m_Pointer->Draw(shader);
	}


	const std::filesystem::path& Model::Path() const
	{
		return m_Pointer->Path();
	}
}