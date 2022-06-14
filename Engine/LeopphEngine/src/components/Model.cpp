#include "Model.hpp"

#include "../rendering/geometry/ModelParser.hpp"

#include <utility>


namespace leopph
{
	Model::Model(std::filesystem::path path) :
		m_Path{std::move(path)}
	{
		SwapRenderable(MeshGroup{internal::ModelParser{}.Parse(m_Path)});
	}


	auto Model::Clone() const -> ComponentPtr<>
	{
		return CreateComponent<Model>(*this);
	}


	auto Model::Path() const noexcept -> std::filesystem::path const&
	{
		return m_Path;
	}
}
