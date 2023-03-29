#include "OpenScenePrompt.hpp"

#include <imgui.h>

namespace leopph::editor {
auto DrawOpenScenePrompt() -> void {
	if (ImGui::Begin("No Open Scene##NoOpenScenePrompt", nullptr, ImGuiWindowFlags_None)) {
		auto const promptTextLabel{ "Create or open a scene from the Project Menu to start editing!" };
		auto const windowSize{ ImGui::GetWindowSize() };
		auto const textSize{ ImGui::CalcTextSize(promptTextLabel) };

		ImGui::SetCursorPosX((windowSize.x - textSize.x) * 0.5f);
		ImGui::SetCursorPosY((windowSize.y - textSize.y) * 0.5f);

		ImGui::Text("%s", promptTextLabel);
	}
	ImGui::End();
}
}
