#pragma once

#include <memory>

#include <sorcery/Core.hpp>
#include <sorcery/host/rendering/view.hpp>
#include <sorcery/rendering/render_target.hpp>


namespace sorcery {
class World;


SORCERYAPI
auto CollectGameViews(World const& world, std::shared_ptr<rendering::RenderTarget> const& default_rt,
                      rendering::ViewList& out) -> void;
}
