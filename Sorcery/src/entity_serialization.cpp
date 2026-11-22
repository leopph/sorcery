#include "sorcery/entity_serialization.hpp"

#include <ranges>
#include <string_view>
#include <unordered_map>

#include <spdlog/spdlog.h>

#include "sorcery/Serialization.hpp"
#include "sorcery/util.hpp"
#include "sorcery/assets/Asset.hpp"
#include "sorcery/host/app.hpp"
#include "sorcery/host/job_system.hpp"
#include "sorcery/host/assets/asset_manager.hpp"
#include "sorcery/scene/Component.hpp"
#include "sorcery/scene/Entity.hpp"
#include "sorcery/scene/SceneObject.hpp"


namespace sorcery {
auto SerializeEntitySet(
  std::span<Entity const* const> const entities,
  EntitySerializationContext const& ctx
) -> YAML::Node {
  auto scene_objects = std::vector<SceneObject const*>{};
  auto components = std::vector<Component const*>{};
  auto scene_obj_to_id = std::unordered_map<SceneObject const*, std::size_t>{};

  for (auto const* const entity : entities) {
    scene_objects.emplace_back(entity);

    for (auto const component : entity->GetComponents(components)) {
      scene_objects.emplace_back(component);
    }
  }

  // Every object gets an ID starting from 1. 0 is reserved for nullptr.

  for (auto const* const obj : scene_objects) {
    scene_obj_to_id[obj] = scene_obj_to_id.size() + 1uz;
  }

  auto const ext_func = [&scene_obj_to_id]
  (rttr::variant const& v, [[maybe_unused]] YamlSerializeContext const& yaml_serialize_ctx) -> YAML::Node {
    auto const variant_type = v.get_type();

    if (!variant_type.is_pointer()) {
      return {};
    }

    // std::remove_cv<T>
    auto const raw_type = variant_type.get_raw_type();

    if (!raw_type.is_derived_from(rttr::type::get<SceneObject>())) {
      return {};
    }

    auto conv_success = false;
    auto const so_ptr = v.convert<SceneObject*>(&conv_success);
    assert(conv_success);

    if (!so_ptr) {
      return YAML::Node{0uz};
    }

    auto const it = scene_obj_to_id.find(so_ptr);

    if (it == scene_obj_to_id.end()) {
      auto const raw_type_name = raw_type.get_name();

      spdlog::warn(
        "Failed to serialize SceneObject reference: referenced object of type {} not included in the serialized set.",
        std::string_view{raw_type_name.data(), raw_type_name.size()}
      );

      return YAML::Node{0uz};
    }

    return YAML::Node{it->second};
  };

  auto ret_node = YAML::Node{};

  auto const yaml_serialize_ctx = YamlSerializeContext{
    .asset_ref_serialization = ctx.asset_ref_serialization
  };

  for (auto const* const obj : scene_objects) {
    auto obj_node = YAML::Node{};
    obj_node["type"] = rttr::type::get(*obj).get_name().to_string();
    obj_node["properties"] = ReflectionSerializeToYaml(*obj, yaml_serialize_ctx, ext_func);
    ret_node.push_back(obj_node);
  }

  return ret_node;
}


auto DeserializeEntitySet(
  YAML::Node const& scene_objects_node,
  YamlDeserializeContext const& ctx
) -> std::vector<std::unique_ptr<Entity>> {
  // Discover all asset references in the scene objects and preload them

  auto required_asset_ids = std::vector<AssetId>{};

  auto const discover_asset_references = [&ctx, &required_asset_ids]
  (this auto& self, YAML::Node const& node, rttr::type const& type) -> void {
    if (!node.IsDefined()) {
      return;
    }

    auto const actual_type = type.is_wrapper() ? type.get_wrapped_type() : type;

    if (node.IsMap() && actual_type.is_pointer()
        && actual_type.get_raw_type().is_derived_from(rttr::type::get<Asset>())) {
      if (auto const asset_id = DeserializeAssetId(node, ctx).value_or(AssetId::Invalid());
        asset_id.IsValid()) {
        required_asset_ids.emplace_back(asset_id);
        return;
      }
    }

    if (node.IsSequence() && actual_type.is_sequential_container()) {
      if (auto const template_args = actual_type.get_template_arguments(); !template_args.empty()) {
        for (auto const& elem : node) {
          self(elem, *template_args.begin());
        }
      }

      return;
    }

    if (node.IsMap() && actual_type.is_class()) {
      for (auto const& prop : actual_type.get_properties()) {
        self(node[prop.get_name().to_string()], prop.get_type());
      }
    }
  };


  // These pointers OWN the objects. A scene/entity set consists of entities and components.
  // Components will be taken ownership of by the entities during deserialization.
  // The scene will take ownership of the entities.
  auto scene_objects = std::vector<SceneObject*>{};

  for (auto const& scene_obj_node : scene_objects_node) {
    auto const type_node = scene_obj_node["type"];
    auto const type_name = type_node.as<std::string>();
    auto const type = rttr::type::get_by_name(type_name);

    if (!type.is_derived_from(rttr::type::get<SceneObject>())) {
      spdlog::warn("Failed to deserialize SceneObject: type {} is not a SceneObject.", type_name);
      scene_objects.emplace_back(nullptr);
      continue;
    }

    auto const scene_obj = ReflCast<SceneObject*>(MakeUniqueObject(type).release());
    discover_asset_references(scene_obj_node["properties"], type);
    scene_objects.emplace_back(scene_obj);
  }

  // Preload all assets used by the new scene objects

  std::ranges::sort(required_asset_ids, std::less{});

  required_asset_ids.erase(std::ranges::unique(required_asset_ids).begin(), required_asset_ids.end());

  auto asset_preload_jobs = std::vector<ObserverPtr<Job>>{};
  asset_preload_jobs.reserve(required_asset_ids.size());

  for (auto const& asset_id : required_asset_ids) {
    asset_preload_jobs.emplace_back(App::Instance().GetJobSystem().CreateJob([](AssetId const& target_asset_id) {
      App::Instance().GetAssetManager().Resolve(target_asset_id);
    }, asset_id));

    App::Instance().GetJobSystem().Submit(asset_preload_jobs.back());
  }

  // Deserialize the scene objects

  auto const ext_func = [&scene_objects]
  (YAML::Node const& obj_node, rttr::variant& v, [[maybe_unused]] YamlDeserializeContext const& yaml_ctx) -> void {
    auto const variant_type = v.get_type();

    if (!variant_type.is_pointer()) {
      return;
    }

    // std::remove_cv<T>
    auto const raw_type = variant_type.get_raw_type();

    if (!raw_type.is_derived_from(rttr::type::get<SceneObject>())) {
      return;
    }

    auto obj_id = 0uz;

    try {
      obj_id = obj_node.as<std::size_t>();
    } catch (...) {
      auto const raw_type_name = raw_type.get_name();
      spdlog::error("Failed to deserialize SceneObject reference: failed to read ID of object of type {}.",
        std::string_view{raw_type_name.data(), raw_type_name.size()});
      return;
    }

    if (obj_id == 0) {
      return;
    }

    auto const obj_idx = obj_id - 1uz;

    if (obj_idx >= scene_objects.size()) {
      auto const raw_type_name = raw_type.get_name();
      spdlog::debug("Failed to deserialize SceneObject reference: ID of object of type {} out of range.",
        std::string_view{raw_type_name.data(), raw_type_name.size()});
      return;
    }

    auto const so = scene_objects[obj_idx];

    if (!so) {
      return;
    }

    auto converted = rttr::variant{so};

    if (!converted.convert(variant_type)) {
      auto const obj_type_name = so->get_type().get_name();
      auto const ref_type_name = variant_type.get_name();
      spdlog::error(
        "Failed to deserialize SceneObject reference: object of type {} is incompatible with reference type {}.",
        std::string_view{obj_type_name.data(), obj_type_name.size()},
        std::string_view{ref_type_name.data(), ref_type_name.size()});
      return;
    }

    v = std::move(converted);
  };

  for (auto const [idx, obj] : std::views::enumerate(scene_objects)) {
    if (obj) {
      ReflectionDeserializeFromYaml(scene_objects_node[idx]["properties"], *obj, ctx, ext_func);
    }
  }

  LogException("Failed to preload asset", App::Instance().GetJobSystem().TryWaitAll(asset_preload_jobs));

  auto ret = std::vector<std::unique_ptr<Entity>>{};

  // Add the new entities to the returned vector

  for (auto* const scene_obj : scene_objects) {
    if (scene_obj) {
      if (auto* const entity = ReflCast<Entity*>(scene_obj)) {
        ret.emplace_back(entity);
      }
    }
  }

  return ret;
}
}
