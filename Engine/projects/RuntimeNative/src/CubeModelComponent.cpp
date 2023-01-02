#include "CubeModelComponent.hpp"

#include <format>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include "Systems.hpp"

namespace leopph {
	Object::Type const CubeModelComponent::SerializationType{Object::Type::CubeModel};

	CubeModelComponent::CubeModelComponent() :
		mMat{ gRenderer.GetDefaultMaterial() } {
		gRenderer.RegisterCubeModel(this);
	}


	CubeModelComponent::~CubeModelComponent() {
		gRenderer.UnregisterCubeModel(this);
	}


	auto CubeModelComponent::GetMaterial() const noexcept -> std::shared_ptr<Material> {
		return mMat;
	}


	auto CubeModelComponent::SetMaterial(std::shared_ptr<Material> material) noexcept -> void {
		if (material) {
			mMat = std::move(material);
		}
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
						SetMaterial(std::static_pointer_cast<Material>(mat->GetSharedPtr()));
						break;
					}
				}

				ImGui::EndPopup();
			}

			ImGui::SameLine();
			ImGui::Text("%s", mMat->GetName().data());

			ImGui::EndTable();
		}
	}

	auto CubeModelComponent::GetSerializationType() const -> Type {
		return Type::CubeModel;
	}


	auto CubeModelComponent::CreateManagedObject() -> MonoObject* {
		return ManagedAccessObject::CreateManagedObject("leopph", "CubeModel");
	}
}
