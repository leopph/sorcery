#include <imgui.h>

#include "ObjectWrappers.hpp"
#include "../MeshImporter.hpp"


namespace leopph::editor {
auto ObjectWrapperFor<Mesh>::OnDrawProperties([[maybe_unused]] Context& context, Object& object) -> void {
  if (auto const& mesh{ dynamic_cast<Mesh&>(object) }; ImGui::BeginTable(std::format("{}", mesh.GetGuid().ToString()).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::PushItemWidth(FLT_MIN);
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(-FLT_MIN);

    ImGui::TableSetColumnIndex(0);
    ImGui::Text("%s", "Vertex Count");

    ImGui::TableNextColumn();
    ImGui::Text("%s", std::to_string(mesh.GetPositions().size()).c_str());

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Index Count");

    ImGui::TableNextColumn();
    ImGui::Text("%s", std::to_string(mesh.GetIndices().size()).c_str());

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Submesh Count");

    ImGui::TableNextColumn();
    ImGui::Text("%s", std::to_string(mesh.GetSubMeshes().size()).c_str());

    ImGui::EndTable();
  }
}


auto ObjectWrapperFor<Mesh>::GetImporter() -> Importer& {
  MeshImporter static meshImporter;
  return meshImporter;
}
}
