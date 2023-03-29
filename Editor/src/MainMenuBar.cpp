#include "MainMenuBar.hpp"

#include <nfd.h>

namespace leopph::editor {
auto DrawMainMenuBar(Context& context, bool& showDemoWindow) -> void {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Open Project")) {
				if (nfdchar_t* selectedPath{ nullptr }; NFD_PickFolder(nullptr, &selectedPath) == NFD_OKAY) {
					context.ExecuteInBusyEditor([selectedPath, &context] {
						context.OpenProject(selectedPath);
						std::free(selectedPath);
					});
				}
			}

			if (ImGui::MenuItem("Save Current Scene")) {
				if (context.GetScene()) {
					context.GetScene()->Save();
					context.SaveRegisteredNativeAsset(*context.GetScene());
				}
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Debug")) {
			if (showDemoWindow) {
				if (ImGui::MenuItem("Hide Demo Window")) {
					showDemoWindow = false;
				}
			}
			else {
				if (ImGui::MenuItem("Show Demo Window")) {
					showDemoWindow = true;
				}
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}
}
