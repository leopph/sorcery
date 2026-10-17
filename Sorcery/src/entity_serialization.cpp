#include "sorcery/entity_serialization.hpp"

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
  std::vector<SceneObject const*> scene_objects;
  std::vector<Component const*> components;
  std::unordered_map<void const*, int> scene_obj_to_id;

  for (auto const* const entity : entities) {
    scene_objects.emplace_back(entity);

    for (auto const component : entity->GetComponents(components)) {
      scene_objects.emplace_back(component);
    }
  }

  for (auto const* const obj : scene_objects) {
    scene_obj_to_id[obj] = static_cast<int>(std::ssize(scene_obj_to_id) + 1);
  }

  auto const ext_func{
    [&scene_obj_to_id](
    rttr::variant const& v,
    [[maybe_unused]] YamlSerializeContext const& yaml_serialize_ctx
  ) -> YAML::Node {
      YAML::Node ret_node;

      if (v.get_type().is_pointer() && v.get_type().get_raw_type().is_derived_from(rttr::type::get<SceneObject>())) {
        auto const it{scene_obj_to_id.find(v.get_value<SceneObject*>())};
        ret_node = it != std::end(scene_obj_to_id) ? it->second : 0;
      }

      return ret_node;
    }
  };

  YAML::Node ret_node;

  YamlSerializeContext const yaml_serialize_ctx{
    .asset_ref_serialization = ctx.asset_ref_serialization
  };

  for (auto const* const obj : scene_objects) {
    YAML::Node obj_node;
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

  std::vector<AssetId> required_asset_ids;

  std::function<void(YAML::Node const&, rttr::type const&)> discover_asset_references;
  discover_asset_references = [&ctx, &discover_asset_references, &required_asset_ids](
    YAML::Node const& node, rttr::type const& type) -> void {
      if (!node.IsDefined()) {
        return;
      }

      auto const actual_type{type.is_wrapper() ? type.get_wrapped_type() : type};

      if (node.IsMap() && actual_type.is_pointer() && actual_type.get_raw_type().is_derived_from(
            rttr::type::get<Asset>())) {
        if (auto const asset_id{DeserializeAssetId(node, ctx).value_or(AssetId::Invalid())};
          asset_id.IsValid()) {
          required_asset_ids.emplace_back(asset_id);
          return;
        }
      }

      if (node.IsSequence() && actual_type.is_sequential_container()) {
        if (auto const template_args{actual_type.get_template_arguments()}; !template_args.empty()) {
          for (auto const& elem : node) {
            discover_asset_references(elem, *template_args.begin());
          }
        }

        return;
      }

      if (node.IsMap() && actual_type.is_class()) {
        for (auto const& prop : actual_type.get_properties()) {
          discover_asset_references(node[prop.get_name().to_string()], prop.get_type());
        }
      }
    };


  // These pointers OWN the objects. A scene/entity set consists of entities and components.
  // Components will be taken ownership of by the entities during deserialization.
  // The scene will take ownership of the entities.
  std::vector<SceneObject*> scene_objects;

  static std::unordered_map<int, SceneObject*> ptr_fix_up;
  ptr_fix_up.clear();

  for (auto const& scene_obj_node : scene_objects_node) {
    auto const type_node{scene_obj_node["type"]};
    auto const type{rttr::type::get_by_name(type_node.as<std::string>())};
    auto const scene_obj{static_cast<SceneObject*>(MakeUniqueObject(type).release())};
    scene_objects.emplace_back(scene_obj);
    ptr_fix_up[static_cast<int>(std::ssize(ptr_fix_up)) + 1] = scene_obj;
    discover_asset_references(scene_obj_node["properties"], type);
  }

  // Preload all assets used by the new scene objects

  std::ranges::sort(required_asset_ids, std::less{});

  required_asset_ids.erase(std::ranges::unique(required_asset_ids).begin(), required_asset_ids.end());

  std::vector<ObserverPtr<Job>> asset_preload_jobs;
  asset_preload_jobs.reserve(required_asset_ids.size());

  for (auto const& asset_id : required_asset_ids) {
    asset_preload_jobs.emplace_back(App::Instance().GetJobSystem().CreateJob([](AssetId const& target_asset_id) {
      App::Instance().GetAssetManager().Resolve(target_asset_id);
    }, asset_id));

    App::Instance().GetJobSystem().Submit(asset_preload_jobs.back());
  }

  // Deserialize the scene objects

  auto const deserialize_scene_obj_ptr{
    [](YAML::Node const& obj_node, rttr::variant& v, [[maybe_unused]] YamlDeserializeContext const& yaml_ctx) -> void {
      if (v.get_type().is_pointer() && v.get_type().get_raw_type().is_derived_from(rttr::type::get<SceneObject>())) {
        if (auto const it{ptr_fix_up.find(obj_node.as<int>(0))}; it != std::end(ptr_fix_up)) {
          auto const type{v.get_type()};
          v = it->second;
          [[maybe_unused]] auto const success{v.convert(type)};
          assert(success);
        }
      }
    }
  };

  for (auto const& [fileId, obj] : ptr_fix_up) {
    ReflectionDeserializeFromYaml(scene_objects_node[fileId - 1]["properties"], *obj,
      ctx, deserialize_scene_obj_ptr);
  }

  LogException("Failed to preload asset", App::Instance().GetJobSystem().TryWaitAll(asset_preload_jobs));

  std::vector<std::unique_ptr<Entity>> ret;

  // Add the new entities to the returned vector

  for (auto* const scene_obj : scene_objects) {
    if (auto* const entity{ReflCast<Entity*>(scene_obj)}) {
      ret.emplace_back(entity);
    }
  }

  return ret;
}
}
