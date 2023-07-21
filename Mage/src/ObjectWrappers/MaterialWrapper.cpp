#include <imgui.h>
#include <imgui_stdlib.h>

#include "ObjectWrappers.hpp"
#include "../EditorContext.hpp"
#include "ResourceManager.hpp"


namespace sorcery::mage {
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
      [callCount = 0]<typename TexGetFn, typename TexSetFn>(std::string_view const label, TexGetFn&& texGetFn, TexSetFn&& texSetFn) mutable requires std::is_invocable_r_v<std::shared_ptr<Texture2D>, TexGetFn> && std::is_invocable_r_v<void, TexSetFn, std::shared_ptr<Texture2D>> {
        auto constexpr nullTexName{ "None" };
        static std::vector<std::weak_ptr<Texture2D>> textures;
        static std::string texFilter;

        ImGui::TableNextColumn();
        ImGui::Text("%s", label.data());
        ImGui::TableNextColumn();

        auto const popupId{ std::format("SelectTexturePopUp{}", callCount) };

        auto const queryTextures{
          [] {
            gResourceManager.FindResourcesOfType(textures);

            std::erase_if(textures, [](std::weak_ptr<Texture2D> const& texWeak) {
              auto const tex{ texWeak.lock() };
              return tex && !Contains(tex->GetName(), texFilter);
            });

            std::ranges::sort(textures, [](std::weak_ptr<Texture2D> const& lhsWeak, std::weak_ptr<Texture2D> const& rhsWeak) {
              auto const lhs{ lhsWeak.lock() };
              auto const rhs{ rhsWeak.lock() };
              return !lhs || (rhs && lhs->GetName() < rhs->GetName());
            });

            textures.insert(std::begin(textures), std::weak_ptr<Texture2D>{});
          }
        };

        if (ImGui::BeginPopup(popupId.data())) {
          if (ImGui::InputText("###SearchTextures", &texFilter)) {
            queryTextures();
          }

          for (auto const texWeak : textures) {
            auto const tex{ texWeak.lock() };
            if (ImGui::Selectable(std::format("{}##texoption{}", tex
                                                                   ? tex->GetName()
                                                                   : nullTexName, tex
                                                                                    ? tex->GetGuid().ToString()
                                                                                    : "0").c_str(), tex == texGetFn())) {
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
        ImGui::Text("%s", texGetFn()
                            ? texGetFn()->GetName().data()
                            : nullTexName);

        callCount += 1;
      }
    };

    drawTextureOption("Albedo Map",
                      [&mtl] {
                        return mtl.GetAlbedoMap().lock();
                      },
                      [&mtl](auto const& tex) -> void {
                        mtl.SetAlbedoMap(tex);
                      });

    drawTextureOption("Metallic Map",
                      [&mtl] {
                        return mtl.GetMetallicMap().lock();
                      },
                      [&mtl](auto const& tex) -> void {
                        mtl.SetMetallicMap(tex);
                      });

    drawTextureOption("Roughness Map",
                      [&mtl]() {
                        return mtl.GetRoughnessMap().lock();
                      },
                      [&mtl](auto const& tex) -> void {
                        mtl.SetRoughnessMap(tex);
                      });

    drawTextureOption("Ambient Occlusion Map",
                      [&mtl] {
                        return mtl.GetAoMap().lock();
                      },
                      [&mtl](auto const& tex) -> void {
                        mtl.SetAoMap(tex);
                      });

    drawTextureOption("Normal Map",
                      [&mtl] {
                        return mtl.GetNormalMap().lock();
                      },
                      [&mtl](auto const& tex) -> void {
                        mtl.SetNormalMap(tex);
                      });

    ImGui::EndTable();
  }

  if (ImGui::Button("Save##SaveMaterialAsset")) {
    context.SaveRegisteredNativeAsset(mtl);
  }
}
}
