#include "ecs/scene_parser.hpp"
#include "ecs/entities.hpp"
#include "renderer/model.hpp"
#include "renderer/font.hpp"
#include "renderer/post/post_processing.hpp"
#include "core/game.hpp"
#include "core/logger.hpp"
#include "config.hpp"

namespace bls
{
    void SceneParser::parse_scene(ECS& ecs, const str& file)
    {
        std::fstream scene(file);

        u32 id = 0;
        str entity_name = "";
        str component_name = "";
        str config_name = "";
        str line = "";
        bool entity_detected = false;
        bool config_detected = false;

        while (std::getline(scene, line))
        {
            // Remove unecessary chars
            line.erase(std::remove(line.begin(), line.end(), '('), line.end());
            line.erase(std::remove(line.begin(), line.end(), ')'), line.end());

            bool insideQuotes = false;
            str processed_line = "";
            for (const char c : line)
            {
                if (c == '~')
                {
                    insideQuotes = !insideQuotes;
                    continue;
                }

                if (!insideQuotes && std::isspace(c))
                    continue;

                processed_line += c;
            }

            line = processed_line;

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

            // Parse configuration
            if (line[0] == '<')
            {
                line.erase(std::remove(line.begin(), line.end(), '<'), line.end());
                line.erase(std::remove(line.begin(), line.end(), '>'), line.end());

                config_name = line;
                config_detected = true;
                continue;
            }

            // Finish parsing object
            else if (line[0] == '}')
            {
                entity_detected = false;
                config_detected = false;
                continue;
            }

            if (entity_detected)
                parse_component(ecs, line, id, entity_name);

            else if (config_detected)
                parse_config(line, config_name);
        }

        scene.close();
        LOG_SUCCESS("scene loaded from '%s'", file.c_str());
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
                write_vec3(&scene, ecs.transforms[id]->position, ", ");
                write_vec3(&scene, ecs.transforms[id]->rotation, ", ");
                write_vec3(&scene, ecs.transforms[id]->scale, ";\n");
            }

            if (ecs.physics_objects.count(id))
            {
                scene << "\tphysics_object: ";
                write_vec3(&scene, ecs.physics_objects[id]->velocity, ", ");
                write_vec3(&scene, ecs.physics_objects[id]->terminal_velocity, ", ");
                write_vec3(&scene, ecs.physics_objects[id]->force, ", ");
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
                    write_vec3(&scene, collider->dimensions, ", ");
                }

                write_vec3(&scene, ecs.colliders[id]->offset, ", ");
                scene << to_str(ecs.colliders[id]->immovable) << ";" << "\n";
            }

            if (ecs.dir_lights.count(id))
            {
                auto& dir_light = *ecs.dir_lights[id];

                scene << "\tdir_light: ";
                write_vec3(&scene, dir_light.ambient, ", ");
                write_vec3(&scene, dir_light.diffuse, ", ");
                write_vec3(&scene, dir_light.specular, ";\n");
            }

            if (ecs.point_lights.count(id))
            {
                auto& point_light = *ecs.point_lights[id];

                scene << "\tpoint_light: ";
                write_vec3(&scene, point_light.ambient, ", ");
                write_vec3(&scene, point_light.diffuse, ", ");
                write_vec3(&scene, point_light.specular, ", ");

                scene << to_str(point_light.constant) << ", ";
                scene << to_str(point_light.linear) << ", ";
                scene << to_str(point_light.quadratic) << ";" << "\n";
            }

            if (ecs.cameras.count(id))
            {
                auto& camera = *ecs.cameras[id];

                scene << "\tcamera: ";
                write_vec3(&scene, camera.target_offset, ", ");
                write_vec3(&scene, camera.world_up, ", ");

                scene << to_str(camera.zoom) << ", ";
                scene << to_str(camera.near) << ", ";
                scene << to_str(camera.far) << ", ";
                scene << to_str(camera.lerp_factor) << ";" << "\n";
            }

            if (ecs.camera_controllers.count(id))
            {
                auto& controller = *ecs.camera_controllers[id];

                scene << "\tcamera_controller: ";

                write_vec3(&scene, controller.speed, ", ");
                scene << to_str(controller.sensitivity) << ";" << "\n";
            }

            if (ecs.texts.count(id))
            {
                auto& text = *ecs.texts[id];

                scene << "\ttext: ";

                scene << text.font_file << ", ";
                scene << "~" << text.text << "~" << ", ";
                write_vec3(&scene, text.color, ";\n");
            }

            if (ecs.sounds.count(id))
            {
                for (const auto& [name, sound] : ecs.sounds[id])
                {
                    scene << "\tsound: ";

                    scene << sound->file << ", ";
                    scene << sound->name << ", ";

                    scene << to_str(sound->volume) << ", ";
                    scene << to_str(sound->play_now) << ", ";
                    scene << to_str(sound->looping) << ";" << "\n";
                }
            }

            if (ecs.timers.count(id))
            {
                scene << "\ttimer: ";
                scene << to_str(0.0) << ";" << "\n";
            }

            if (ecs.transform_animations.count(id))
            {
                auto& key_frames = ecs.transform_animations[id]->key_frames;
                scene << "\ttransform_animation: ";
                scene << to_str(key_frames.size()) << ", ";

                for (auto& key_frame : key_frames)
                {
                    write_vec3(&scene, key_frame.transform.position, ", ");
                    write_vec3(&scene, key_frame.transform.rotation, ", ");
                    write_vec3(&scene, key_frame.transform.scale, ", ");

                    scene << to_str(key_frame.duration) << "; ";
                }
                scene << "\n";
            }

            scene << "}" << "\n\n";
        }

        scene.close();
        LOG_SUCCESS("current scene saved to '%s'", file);
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
            vec3 velocity, terminal_velocity, force;

            velocity = read_vec3(&iline, ',');
            terminal_velocity = read_vec3(&iline, ',');
            force = read_vec3(&iline, ',');
            std::getline(iline, mass, ';');

            ecs.physics_objects[entity_id] = std::make_unique<PhysicsObject>(PhysicsObject(velocity, terminal_velocity, force, stof(mass)));
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
                LOG_ERROR("invalid collider type");
        }

        else if (component_name == "dir_light")
        {
            vec3 ambient, diffuse, specular;

            ambient = read_vec3(&iline, ',');
            diffuse = read_vec3(&iline, ',');
            specular = read_vec3(&iline, ',');

            ecs.dir_lights[entity_id] = std::make_unique<DirectionalLight>(DirectionalLight(ambient, diffuse, specular));
        }

        else if (component_name == "point_light")
        {
            vec3 ambient, diffuse, specular;
            str constant, linear, quadratic;

            ambient = read_vec3(&iline, ',');
            diffuse = read_vec3(&iline, ',');
            specular = read_vec3(&iline, ',');

            std::getline(iline, constant, ',');
            std::getline(iline, linear, ',');
            std::getline(iline, quadratic, ';');

            ecs.point_lights[entity_id] = std::make_unique<PointLight>(PointLight(ambient, diffuse, specular,
                                          stof(constant), stof(linear), stof(quadratic)));
        }

        else if (component_name == "camera")
        {
            vec3 offset, world_up;
            str zoom, near, far, lerp_factor;

            offset = read_vec3(&iline, ',');
            world_up = read_vec3(&iline, ',');

            std::getline(iline, zoom, ',');
            std::getline(iline, near, ',');
            std::getline(iline, far, ',');
            std::getline(iline, lerp_factor, ';');

            ecs.cameras[entity_id] = std::make_unique<Camera>(Camera(offset, world_up,
                                     stof(zoom), stof(near), stof(far), stof(lerp_factor)));
        }

        else if (component_name == "camera_controller")
        {
            str sensitivity;
            vec3 speed;

            speed = read_vec3(&iline, ',');
            std::getline(iline, sensitivity, ';');

            ecs.camera_controllers[entity_id] = std::make_unique<CameraController>(CameraController(speed, stof(sensitivity)));
        }

        else if (component_name == "text")
        {
            str font_file, text;
            vec3 color;

            std::getline(iline, font_file, ',');
            std::getline(iline, text, ',');
            color = read_vec3(&iline, ',');

            auto font = Font::create(entity_name, font_file);
            ecs.texts[entity_id] = std::make_unique<Text>(Text(font.get(), font_file, text, color));
        }

        else if (component_name == "sound")
        {
            str sound_file, sound_name, volume, play_now, looping;

            std::getline(iline, sound_file, ',');
            std::getline(iline, sound_name, ',');
            std::getline(iline, volume, ',');
            std::getline(iline, play_now, ',');
            std::getline(iline, looping, ';');

            Game::get().get_audio_engine().load(sound_name, sound_file, stoi(looping));
            ecs.sounds[entity_id][sound_name] = std::make_unique<Sound>(Sound(sound_file, sound_name, stof(volume), stoi(play_now), stoi(looping)));
        }

        else if (component_name == "timer")
        {
            str time;
            std::getline(iline, time, ';');
            ecs.timers[entity_id] = std::make_unique<Timer>(Timer(stof(time)));
        }

        else if (component_name == "transform_animation")
        {
            vec3 position, rotation, scale;
            str size, duration;

            std::getline(iline, size, ',');
            std::vector<KeyFrame> key_frames(stoi(size));

            for (u16 i = 0; i < stoi(size); i++)
            {
                position = read_vec3(&iline, ',');
                rotation = read_vec3(&iline, ',');
                scale = read_vec3(&iline, ',');
                std::getline(iline, duration, ';');

                Transform transform = { position, rotation, scale };
                key_frames[i] = { transform, stof(duration) };
            }

            ecs.transform_animations[entity_id] = std::make_unique<TransformAnimation>(key_frames);
        }
    }

    void SceneParser::parse_config(const str& line, const str& config_name)
    {
        if (config_name == "post_processing")
        {
            str parameter;
            std::istringstream iline(line);
            std::getline(iline, parameter, ':');
            
            str position, enabled;
            std::getline(iline, position, ',');
            std::getline(iline, enabled, ',');

            // FogPass
            if (parameter == "fog_pass")
            {
                vec3 fog_color = read_vec3(&iline, ',');
                vec2 min_max = read_vec2(&iline, ',');

                for (auto& pass : AppConfig::render_passes)
                {
                    if (typeid(*pass.pass) == typeid(FogPass))
                    {
                        auto* fog_pass = static_cast<FogPass*>(pass.pass);
                        fog_pass->fog_color = fog_color;
                        fog_pass->min_max = min_max;
                        break;
                    }
                }
            }

            else
                std::cout << "@TODO Other passes\n";
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

    vec2 SceneParser::read_vec2(std::istringstream* iline, char delimiter)
    {
        vec2 vec;
        str buffer;

        // Read position
        for (u16 i = 0; i < 2; i++)
        {
            std::getline(*iline, buffer, delimiter);
            vec[i] = stof(buffer);
        }

        return vec;
    }

    void SceneParser::write_vec3(std::ofstream* scene, const vec3& vec, const str& end_str)
    {
        *scene << to_str(vec).substr(4) << end_str;
    }
};
