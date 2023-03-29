#include "PerformanceCounterWindow.hpp"

#include <chrono>
#include <imgui.h>
#include <implot.h>

#include "Timing.hpp"

namespace leopph::editor {
auto DrawPerformanceCounterWindow() -> void {
	if (ImGui::Begin("Performance")) {
		std::chrono::duration<float, std::ratio<1>> const frameTimeSeconds{ timing::GetFrameTime() };
		std::chrono::duration<float, std::milli> const frameTimeMillis{ frameTimeSeconds };

		auto constexpr static MAX_DATA_POINT_COUNT{ 500 };
		std::vector<float> static dataPoints;

		if (dataPoints.size() == MAX_DATA_POINT_COUNT) {
			dataPoints.erase(std::begin(dataPoints));
		}

		dataPoints.emplace_back(frameTimeMillis.count());

		ImGui::Text("%d FPS", static_cast<int>(1.0f / frameTimeSeconds.count()));
		ImGui::Text("%.2f ms", static_cast<double>(frameTimeMillis.count()));

		if (ImPlot::BeginPlot("###frameTimeChart", ImGui::GetContentRegionAvail(), ImPlotFlags_NoInputs | ImPlotFlags_NoFrame)) {
			ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, static_cast<double>(*std::ranges::max_element(dataPoints)), ImPlotCond_Always);
			ImPlot::SetupAxisLimits(ImAxis_X1, 0, static_cast<double>(dataPoints.size()), ImPlotCond_Always);
			ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoDecorations);
			ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_NoTickMarks);
			ImPlot::SetupAxisFormat(ImAxis_Y1, "%.0f ms");
			ImPlot::PlotLine("###frameTimeLine", dataPoints.data(), static_cast<int>(dataPoints.size()));
			ImPlot::EndPlot();
		}
	}

	ImGui::End();
}
}
