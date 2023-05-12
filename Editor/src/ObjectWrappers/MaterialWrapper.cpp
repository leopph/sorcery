#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include "ObjectWrappers.hpp"
#include "../MaterialImporter.hpp"
#include "../EditorContext.hpp"


namespace leopph::editor {
auto ObjectWrapperFor<Material>::OnDrawProperties([[maybe_unused]] Context& context, Object& object) -> void {
  auto& mtl{ dynamic_cast<Material&>(object) };

  if (ImGui::BeginTable(std::format("{}", mtl.GetGuid().ToString()).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::PushItemWidth(FLT_MIN);
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(-FLT_MIN);

    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Albedo Color");
    ImGui::TableNextColumn();

    if (Vector3 albedoColor{ mtl.GetAlbedoVector() }; ImGui::ColorEdit3("###matAlbedoColor", albedoColor.GetData())) {
      mtl.SetAlbedoVector(albedoColor);
    }

    ImGui::TableNextColumn();
    ImGui::Text("Metallic");
    ImGui::TableNextColumn();

    if (f32 metallic{ mtl.GetMetallic() }; ImGui::SliderFloat("###matMetallic", &metallic, 0.0f, 1.0f)) {
      mtl.SetMetallic(metallic);
    }

    ImGui::TableNextColumn();
    ImGui::Text("Roughness");
    ImGui::TableNextColumn();

    if (f32 roughness{ mtl.GetRoughness() }; ImGui::SliderFloat("###matRoughness", &roughness, 0.0f, 1.0f)) {
      mtl.SetRoughness(roughness);
    }

    ImGui::TableNextColumn();
    ImGui::Text("Ambient Occlusion");
    ImGui::TableNextColumn();

    if (f32 ao{ mtl.GetAo() }; ImGui::SliderFloat("###matAo", &ao, 0.0f, 1.0f)) {
      mtl.SetAo(ao);
    }

    auto drawTextureOption{
      [callCount = 0](std::string_view const label, auto&& texGetFn, auto&& texSetFn) mutable {
        auto constexpr nullTexName{ "None" };
        static std::vector<Texture2D*> textures;
        static std::string texFilter;

        ImGui::TableNextColumn();
        ImGui::Text("%s", label.data());
        ImGui::TableNextColumn();

        auto const popupId{ std::format("SelectTexturePopUp{}", callCount) };

        auto const queryTextures{
          [] {
            Object::FindObjectsOfType(textures);

            std::erase_if(textures, [](Texture2D const* tex) {
              return tex && !Contains(tex->GetName(), texFilter);
            });

            std::ranges::sort(textures, [](Texture2D const* const left, Texture2D const* const right) {
              return left->GetName() < right->GetName();
            });

            textures.insert(std::begin(textures), nullptr);
          }
        };

        if (ImGui::BeginPopup(popupId.data())) {
          if (ImGui::InputText("###SearchTextures", &texFilter)) {
            queryTextures();
          }

          for (auto const tex : textures) {
            if (ImGui::Selectable(std::format("{}##texoption{}", tex ? tex->GetName() : nullTexName, tex ? tex->GetGuid().ToString() : "0").c_str(), tex == texGetFn())) {
              texSetFn(tex);
              break;
            }
          }

          ImGui::EndPopup();
        }

        if (ImGui::Button(std::format("Select##SelectTextureButton{}", callCount).data())) {
          texFilter.clear();
          queryTextures();
          ImGui::OpenPopup(popupId.data());
        }

        ImGui::SameLine();
        ImGui::Text("%s", texGetFn() ? texGetFn()->GetName().data() : nullTexName);

        callCount += 1;
      }
    };

    drawTextureOption("Albedo Map",
                      [&mtl]() -> Texture2D* {
                        return mtl.GetAlbedoMap();
                      },
                      [&mtl](Texture2D* const tex) -> void {
                        mtl.SetAlbedoMap(tex);
                      });

    drawTextureOption("Metallic Map",
                      [&mtl]() -> Texture2D* {
                        return mtl.GetMetallicMap();
                      },
                      [&mtl](Texture2D* const tex) -> void {
                        mtl.SetMetallicMap(tex);
                      });

    drawTextureOption("Roughness Map",
                      [&mtl]() -> Texture2D* {
                        return mtl.GetRoughnessMap();
                      },
                      [&mtl](Texture2D* const tex) -> void {
                        mtl.SetRoughnessMap(tex);
                      });

    drawTextureOption("Ambient Occlusion Map",
                      [&mtl]() -> Texture2D* {
                        return mtl.GetAoMap();
                      },
                      [&mtl](Texture2D* const tex) -> void {
                        mtl.SetAoMap(tex);
                      });

    drawTextureOption("Normal Map",
                      [&mtl]() -> Texture2D* {
                        return mtl.GetNormalMap();
                      },
                      [&mtl](Texture2D* const tex) -> void {
                        mtl.SetNormalMap(tex);
                      });

    ImGui::EndTable();
  }

  if (ImGui::Button("Save##SaveMaterialAsset")) {
    context.SaveRegisteredNativeAsset(mtl);
  }
}


auto ObjectWrapperFor<Material>::GetImporter() -> Importer& {
  MaterialImporter static mtlImporter;
  return mtlImporter;
}
}
