#include "OnGui.hpp"

#include <imgui.h>

namespace leopph {
	auto SetImGuiContext(ImGuiContext* context) -> void {
		ImGui::SetCurrentContext(context);
	}
}
