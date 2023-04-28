#include "ProjectSettingsWindow.hpp"
#include "Renderer.hpp"

#include <imgui.h>

#include <format>


namespace leopph::editor {
std::string_view const PROJECT_SETTINGS_WINDOW_TITLE{ "Project Settings" };


auto DrawProjectSettingsWindow(bool& isOpen) -> void {
	//ImGui::SetNextWindowSizeConstraints(ImVec2{ 300, 300 }, ImVec2{ -1, -1 });

	if (!ImGui::Begin((std::string{ PROJECT_SETTINGS_WINDOW_TITLE } + "##Window").data(), &isOpen)) {
		ImGui::End();
		return;
	}

	if (!ImGui::BeginTable("ProjectSettingsWindowTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit)) {
		ImGui::End();
		return;
	}

	static int selectedSubMenu{ 0 };

	ImGui::TableNextColumn();
	if (ImGui::BeginChild("ProjectSettingsLeftChild")) {
		ImGui::MenuItem("Graphics");
	}
	ImGui::EndChild();

	ImGui::TableNextColumn();
	if (ImGui::BeginChild("ProjectSettingsRightChild")) {
		if (selectedSubMenu == 0) {
			ImGui::Text("Shadow Distance");
			ImGui::SameLine();

			float shadowDistance{ renderer::GetShadowDistance() };
			if (ImGui::InputFloat("##shadowDistanceInput", &shadowDistance, 0, 0, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue)) {
				renderer::SetShadowDistance(shadowDistance);
			}

			ImGui::Text("Shadow Filtering Mode");
			ImGui::SameLine();

			auto constexpr shadowFilteringModeNames{
				[] {
					std::array<char const*, 6> ret{};
					ret[static_cast<int>(renderer::ShadowFilteringMode::None)] = "No Filtering";
					ret[static_cast<int>(renderer::ShadowFilteringMode::HardwarePCF)] = "PCF 2x2 (hardware)";
					ret[static_cast<int>(renderer::ShadowFilteringMode::PCF3x3)] = "PCF 3x3 (4 taps)";
					ret[static_cast<int>(renderer::ShadowFilteringMode::PCFTent3x3)] = "PCF Tent 3x3 (4 taps)";
					ret[static_cast<int>(renderer::ShadowFilteringMode::PCFTent5x5)] = "PCF Tent 5x5 (9 taps)";
					ret[static_cast<int>(renderer::ShadowFilteringMode::PCSS)] = "PCSS";
					return ret;
				}()
			};
			int currentShadowFilteringModeIdx{ static_cast<int>(renderer::GetShadowFilteringMode()) };
			if (ImGui::Combo("##ShadowFilteringModeCombo", &currentShadowFilteringModeIdx, shadowFilteringModeNames.data(), static_cast<int>(std::ssize(shadowFilteringModeNames)))) {
				SetShadowFilteringMode(static_cast<renderer::ShadowFilteringMode>(currentShadowFilteringModeIdx));
			}

			ImGui::Text("Visualize Shadow Cascades");
			ImGui::SameLine();

			bool visualizeShadowCascades{ renderer::IsVisualizingShadowCascades() };
			if (ImGui::Checkbox("##VisualizeShadowCascadesCheckbox", &visualizeShadowCascades)) {
				renderer::VisualizeShadowCascades(visualizeShadowCascades);
			}

			ImGui::Text("Stable Shadow Cascade Projection");
			ImGui::SameLine();

			bool isUsingStableShadowCascadeProjection{ renderer::IsUsingStableShadowCascadeProjection() };
			if (ImGui::Checkbox("##StableShadowCascadeProjectionCheckbox", &isUsingStableShadowCascadeProjection)) {
				renderer::UseStableShadowCascadeProjection(isUsingStableShadowCascadeProjection);
			}

			ImGui::Text("Shadow Cascade Count");
			ImGui::SameLine();

			int cascadeCount{ renderer::GetShadowCascadeCount() };
			if (ImGui::SliderInt("##cascadeCountInput", &cascadeCount, 1, renderer::GetMaxShadowCascadeCount(), "%d", ImGuiSliderFlags_NoInput)) {
				renderer::SetShadowCascadeCount(cascadeCount);
			}

			auto const cascadeSplits{ renderer::GetNormalizedShadowCascadeSplits() };
			auto const splitCount{ std::ssize(cascadeSplits) };

			for (int i = 0; i < splitCount; i++) {
				ImGui::Text("Split %d (percent)", i + 1);
				ImGui::SameLine();

				float cascadeSplit{ cascadeSplits[i] * 100.0f };

				if (ImGui::SliderFloat(std::format("##cascadeSplit {} input", i).data(), &cascadeSplit, 0, 100, "%.3f", ImGuiSliderFlags_NoInput)) {
					renderer::SetNormalizedShadowCascadeSplit(i, cascadeSplit / 100.0f);
				}
			}
		}
	}
	ImGui::EndChild();

	ImGui::EndTable();
	ImGui::End();
}
}
