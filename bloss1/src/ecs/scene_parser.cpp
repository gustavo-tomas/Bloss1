#include "ecs/scene_parser.hpp"
#include "ecs/entities.hpp"
#include "renderer/model.hpp"

namespace bls
{
    void SceneParser::parse_scene(ECS& ecs, const str& file)
    {
        std::fstream scene(file);

        u32 id = 0;
        str entity_name = "";
        str component_name = "";
        str line = "";
        bool entity_detected = false;

        while (std::getline(scene, line))
        {
            // Remove unecessary chars
            line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
            line.erase(std::remove(line.begin(), line.end(), '('), line.end());
            line.erase(std::remove(line.begin(), line.end(), ')'), line.end());

            // Parse entity
            if (line[0] == '[')
            {
                line.erase(std::remove(line.begin(), line.end(), '['), line.end());
                line.erase(std::remove(line.begin(), line.end(), ']'), line.end());

                id = empty_entity(ecs);
                ecs.names[id] = line;

                entity_name = line;
                entity_detected = true;
                continue;
            }

            // Finish parsing entity
            else if (line[0] == '}')
            {
                entity_detected = false;
                continue;
            }

            if (entity_detected)
                parse_component(ecs, line, id, entity_name);
        }

        scene.close();
    }

    void SceneParser::save_scene(ECS& ecs, const str& file)
    {
        std::ofstream scene(file);

        for (const auto& [id, name] : ecs.names)
        {
            scene << "[" << name << "]" << "\n";
            scene << "{" << "\n";

            if (ecs.models.count(id))
            {
                scene << "\tmodel: ";
                scene << ecs.models[id]->model->path << ", ";
                scene << ecs.models[id]->model->flip_uvs << ";" << "\n";
            }

            if (ecs.transforms.count(id))
            {
                scene << "\ttransform: ";
                scene << to_str(ecs.transforms[id]->position).substr(4) << ", ";
                scene << to_str(ecs.transforms[id]->rotation).substr(4) << ", ";
                scene << to_str(ecs.transforms[id]->scale).substr(4)    << ";" << "\n";
            }

            if (ecs.physics_objects.count(id))
            {
                scene << "\tphysics_object: ";
                scene << to_str(ecs.physics_objects[id]->velocity).substr(4) << ", ";
                scene << to_str(ecs.physics_objects[id]->force).substr(4) << ", ";
                scene << to_str(ecs.physics_objects[id]->mass) << ";" << "\n";
            }

            if (ecs.colliders.count(id))
            {
                auto type = ecs.colliders[id]->type;

                scene << "\tcollider: ";
                scene << Collider::get_collider_str(type) << ", ";

                if (type == Collider::ColliderType::Sphere)
                {
                    auto collider = static_cast<SphereCollider*>(ecs.colliders[id].get());
                    scene << to_str(collider->radius) << ", ";
                }

                else if (type == Collider::ColliderType::Box)
                {
                    auto collider = static_cast<BoxCollider*>(ecs.colliders[id].get());
                    scene << to_str(collider->dimensions).substr(4) << ", ";
                }

                scene << to_str(ecs.colliders[id]->offset).substr(4) << ", ";
                scene << to_str(ecs.colliders[id]->immovable) << ";" << "\n";
            }

            scene << "}" << "\n\n";
        }
    }

    void SceneParser::parse_component(ECS& ecs, const str& line, u32 entity_id, const str& entity_name)
    {
        // Parse component
        str component_name;
        std::istringstream iline(line);
        std::getline(iline, component_name, ':');

        if (component_name == "model")
        {
            str file = "", flip_uvs = "";

            std::getline(iline, file, ',');
            std::getline(iline, flip_uvs, ';');

            auto model = Model::create(entity_name, file, stoi(flip_uvs));

            ecs.models[entity_id] = std::make_unique<ModelComponent>(model.get());
            ecs.transforms[entity_id] = std::make_unique<Transform>();
        }

        else if (component_name == "transform")
        {
            vec3 position, rotation, scale;

            // Read position
            position = read_vec3(&iline, ',');
            rotation = read_vec3(&iline, ',');
            scale = read_vec3(&iline, ',');

            ecs.transforms[entity_id] = std::make_unique<Transform>(Transform(position, rotation, scale));
        }

        else if (component_name == "physics_object")
        {
            str mass;
            vec3 velocity, force;

            velocity = read_vec3(&iline, ',');
            force = read_vec3(&iline, ',');
            std::getline(iline, mass, ';');

            ecs.physics_objects[entity_id] = std::make_unique<PhysicsObject>(PhysicsObject(velocity, force, stof(mass)));
        }

        else if (component_name == "collider")
        {
            str type, immovable;
            vec3 offset;

            std::getline(iline, type, ',');

            if (type == "sphere")
            {
                str radius;

                std::getline(iline, radius, ',');
                offset = read_vec3(&iline, ',');
                std::getline(iline, immovable, ';');

                ecs.colliders[entity_id] = std::make_unique<SphereCollider>(stof(radius), offset, stoi(immovable));
            }

            else if (type == "box")
            {
                str width, height, depth;
                vec3 dimensions = read_vec3(&iline, ',');

                offset = read_vec3(&iline, ',');
                std::getline(iline, immovable, ';');

                ecs.colliders[entity_id] = std::make_unique<BoxCollider>(dimensions, offset, stoi(immovable));
            }

            else
                std::cerr << "invalid collider type\n";
        }
    }

    vec3 SceneParser::read_vec3(std::istringstream* iline, char delimiter)
    {
        vec3 vec;
        str buffer;

        // Read position
        for (u16 i = 0; i < 3; i++)
        {
            std::getline(*iline, buffer, delimiter);
            vec[i] = stof(buffer);
        }

        return vec;
    }
};
