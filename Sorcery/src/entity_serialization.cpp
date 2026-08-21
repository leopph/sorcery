#include "entity_serialization.hpp"

#include "app.hpp"
#include "job_system.hpp"
#include "Serialization.hpp"
#include "resources/Resource.hpp"
#include "scene_objects/Component.hpp"
#include "scene_objects/Entity.hpp"
#include "scene_objects/SceneObject.hpp"


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
    .resource_ref_serialization = ctx.resource_ref_serialization
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
  // Discover all resource references in the scene objects and preload them

  std::vector<ResourceId> required_resource_ids;

  std::function<void(YAML::Node const&, rttr::type const&)> discover_resource_references;
  discover_resource_references = [&ctx, &discover_resource_references, &required_resource_ids](
    YAML::Node const& node, rttr::type const& type) -> void {
      if (!node.IsDefined()) {
        return;
      }

      auto const actual_type{type.is_wrapper() ? type.get_wrapped_type() : type};

      if (node.IsMap() && actual_type.is_pointer() && actual_type.get_raw_type().is_derived_from(
            rttr::type::get<Resource>())) {
        if (auto const res_id{DeserializeResourceId(node, ctx).value_or(ResourceId::Invalid())};
          res_id.IsValid()) {
          required_resource_ids.emplace_back(res_id);
          return;
        }
      }

      if (node.IsSequence() && actual_type.is_sequential_container()) {
        if (auto const template_args{actual_type.get_template_arguments()}; !template_args.empty()) {
          for (auto const& elem : node) {
            discover_resource_references(elem, *template_args.begin());
          }
        }

        return;
      }

      if (node.IsMap() && actual_type.is_class()) {
        for (auto const& prop : actual_type.get_properties()) {
          discover_resource_references(node[prop.get_name().to_string()], prop.get_type());
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
    auto const scene_obj{static_cast<SceneObject*>(Create(type).release())};
    scene_objects.emplace_back(scene_obj);
    ptr_fix_up[static_cast<int>(std::ssize(ptr_fix_up)) + 1] = scene_obj;
    discover_resource_references(scene_obj_node["properties"], type);
  }

  // Preload all resources used by the new scene objects

  std::ranges::sort(required_resource_ids, std::less{});

  required_resource_ids.erase(std::ranges::unique(required_resource_ids).begin(), required_resource_ids.end());

  std::vector<ObserverPtr<Job>> resource_loading_jobs;
  resource_loading_jobs.reserve(required_resource_ids.size());

  for (auto const& res_id : required_resource_ids) {
    resource_loading_jobs.emplace_back(App::Instance().GetJobSystem().CreateJob([](ResourceId const& target_res_id) {
      App::Instance().GetResourceManager().GetOrLoad<Resource>(target_res_id);
    }, res_id));

    App::Instance().GetJobSystem().Run(resource_loading_jobs.back());
  }

  // Deserialize the scene objects

  auto const deserialize_scene_obj_ptr{
    [](YAML::Node const& obj_node, rttr::variant& v, [[maybe_unused]] YamlDeserializeContext const& ctx) -> void {
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

  for (auto const job : resource_loading_jobs) {
    App::Instance().GetJobSystem().Wait(job);
  }

  std::vector<std::unique_ptr<Entity>> ret;

  // Add the new entities to the returned vector

  for (auto* const scene_obj : scene_objects) {
    if (auto* const entity{rttr::rttr_cast<Entity*>(scene_obj)}) {
      ret.emplace_back(entity);
    }
  }

  return ret;
}
}
