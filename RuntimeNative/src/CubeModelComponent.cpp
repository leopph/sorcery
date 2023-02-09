#include "CubeModelComponent.hpp"

#include <format>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include "Systems.hpp"

namespace leopph {
Object::Type const CubeModelComponent::SerializationType{ Object::Type::CubeModel };

CubeModelComponent::CubeModelComponent() :
	mMat{ gRenderer.GetDefaultMaterial().get() },
	mMesh{ gRenderer.GetCubeMesh().get() } {
	gRenderer.RegisterCubeModel(this);
}


CubeModelComponent::~CubeModelComponent() {
	gRenderer.UnregisterCubeModel(this);
}


auto CubeModelComponent::GetMaterial() const noexcept -> Material& {
	return *mMat;
}


auto CubeModelComponent::SetMaterial(Material& material) noexcept -> void {
	mMat = &material;
}

auto CubeModelComponent::GetMesh() const noexcept -> Mesh& {
	return *mMesh;
}

auto CubeModelComponent::SetMesh(Mesh& mesh) noexcept -> void {
	mMesh = &mesh;
}

auto CubeModelComponent::OnGui() -> void {
	if (ImGui::BeginTable(std::format("{}", GetGuid().ToString()).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::PushItemWidth(FLT_MIN);
		ImGui::TableSetColumnIndex(1);
		ImGui::PushItemWidth(-FLT_MIN);

		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Material");
		ImGui::TableNextColumn();

		static std::vector<Material*> materials;
		static std::string matFilter;

		if (ImGui::Button("Select##SelectMaterialForCubeModel")) {
			FindObjectsOfType(materials);
			matFilter.clear();
			ImGui::OpenPopup("ChooseMaterialForCubeModel");
		}

		if (ImGui::BeginPopup("ChooseMaterialForCubeModel")) {
			if (ImGui::InputText("###SearchMat", &matFilter)) {
				FindObjectsOfType(materials);
				std::erase_if(materials, [](Material const* mat) {
					return !mat->GetName().contains(matFilter);
				});
			}

			for (auto const mat : materials) {
				if (ImGui::Selectable(std::format("{}##matoption{}", mat->GetName(), mat->GetGuid().ToString()).c_str())) {
					SetMaterial(*mat);
					break;
				}
			}

			ImGui::EndPopup();
		}

		ImGui::SameLine();
		ImGui::Text("%s", mMat->GetName().data());

		ImGui::TableNextColumn();
		ImGui::Text("%s", "Mesh");
		ImGui::TableNextColumn();

		static std::vector<Mesh*> meshes;
		static std::string meshFilter;

		if (ImGui::Button("Select##SelectMeshForStaticMeshComponent")) {
			FindObjectsOfType(meshes);
			meshFilter.clear();
			ImGui::OpenPopup("ChooseMeshForStaticMeshComponent");
		}

		if (ImGui::BeginPopup("ChooseMeshForStaticMeshComponent")) {
			if (ImGui::InputText("###SearchMesh", &meshFilter)) {
				FindObjectsOfType(meshes);
				std::erase_if(meshes, [](Mesh const* mesh) {
					return !mesh->GetName().contains(meshFilter);
				});
			}

			for (auto const mesh : meshes) {
				if (ImGui::Selectable(std::format("{}##meshoption{}", mesh->GetName(), mesh->GetGuid().ToString()).c_str())) {
					SetMesh(*mesh);
					break;
				}
			}

			ImGui::EndPopup();
		}

		ImGui::SameLine();

		if (mMesh) {
			ImGui::Text("%s", mMesh->GetName().data());
		}

		ImGui::EndTable();
	}
}

auto CubeModelComponent::GetSerializationType() const -> Type {
	return Type::CubeModel;
}


auto CubeModelComponent::CreateManagedObject() -> void {
	return ManagedAccessObject::CreateManagedObject("leopph", "CubeModel");
}
}
