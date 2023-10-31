#include "ecs/scene_parser.hpp"

#include "config.hpp"
#include "core/game.hpp"
#include "core/logger.hpp"
#include "ecs/entities.hpp"
#include "renderer/font.hpp"
#include "renderer/model.hpp"
#include "renderer/post/post_processing.hpp"

namespace bls
{
    void SceneParser::parse_scene(ECS &ecs, const str &file)
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

                if (!insideQuotes && std::isspace(c)) continue;

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

    void SceneParser::save_scene(ECS &ecs, const str &file)
    {
        std::ofstream scene(file);

        for (const auto &[id, entity_name] : ecs.names)
        {
            scene << "[" << entity_name << "]"
                  << "\n";
            scene << "{"
                  << "\n";

            if (ecs.models.count(id))
            {
                scene << "\tmodel: ";
                scene << ecs.models[id]->model->path << ", ";
                scene << ecs.models[id]->model->flip_uvs << ";"
                      << "\n";
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
                scene << to_str(ecs.physics_objects[id]->mass) << ";"
                      << "\n";
            }

            if (ecs.colliders.count(id))
            {
                auto type = ecs.colliders[id]->type;

                scene << "\tcollider: ";
                scene << Collider::get_collider_str(type) << ", ";

                if (type == Collider::ColliderType::Sphere)
                {
                    auto collider = static_cast<SphereCollider *>(ecs.colliders[id].get());
                    scene << to_str(collider->radius) << ", ";
                }

                else if (type == Collider::ColliderType::Box)
                {
                    auto collider = static_cast<BoxCollider *>(ecs.colliders[id].get());
                    write_vec3(&scene, collider->dimensions, ", ");
                }

                write_vec3(&scene, ecs.colliders[id]->offset, ", ");
                scene << to_str(ecs.colliders[id]->immovable) << ", ";
                scene << to_str(ecs.colliders[id]->description_mask) << ", ";
                scene << to_str(ecs.colliders[id]->interaction_mask) << ";"
                      << "\n";
            }

            if (ecs.dir_lights.count(id))
            {
                auto &dir_light = *ecs.dir_lights[id];

                scene << "\tdir_light: ";
                write_vec3(&scene, dir_light.ambient, ", ");
                write_vec3(&scene, dir_light.diffuse, ", ");
                write_vec3(&scene, dir_light.specular, ";\n");
            }

            if (ecs.point_lights.count(id))
            {
                auto &point_light = *ecs.point_lights[id];

                scene << "\tpoint_light: ";
                write_vec3(&scene, point_light.ambient, ", ");
                write_vec3(&scene, point_light.diffuse, ", ");
                write_vec3(&scene, point_light.specular, ", ");

                scene << to_str(point_light.constant) << ", ";
                scene << to_str(point_light.linear) << ", ";
                scene << to_str(point_light.quadratic) << ";"
                      << "\n";
            }

            if (ecs.cameras.count(id))
            {
                auto &camera = *ecs.cameras[id];

                scene << "\tcamera: ";
                write_vec3(&scene, camera.target_offset, ", ");
                write_vec3(&scene, camera.world_up, ", ");

                scene << to_str(camera.zoom) << ", ";
                scene << to_str(camera.near) << ", ";
                scene << to_str(camera.far) << ", ";
                scene << to_str(camera.lerp_factor) << ";"
                      << "\n";
            }

            if (ecs.camera_controllers.count(id))
            {
                auto &controller = *ecs.camera_controllers[id];

                scene << "\tcamera_controller: ";

                write_vec3(&scene, controller.speed, ", ");
                scene << to_str(controller.sensitivity) << ";"
                      << "\n";
            }

            if (ecs.texts.count(id))
            {
                auto &text = *ecs.texts[id];

                scene << "\ttext: ";

                scene << text.font_file << ", ";
                scene << "~" << text.text << "~"
                      << ", ";
                write_vec3(&scene, text.color, ";\n");
            }

            if (ecs.sounds.count(id))
            {
                for (const auto &[name, sound] : ecs.sounds[id])
                {
                    scene << "\tsound: ";

                    scene << sound->file << ", ";
                    scene << sound->name << ", ";

                    scene << to_str(sound->volume) << ", ";
                    scene << to_str(sound->play_now) << ", ";
                    scene << to_str(sound->looping) << ";"
                          << "\n";
                }
            }

            if (ecs.timers.count(id))
            {
                scene << "\ttimer: ";
                scene << to_str(0.0) << ";"
                      << "\n";
            }

            if (ecs.transform_animations.count(id))
            {
                auto &key_frames = ecs.transform_animations[id]->key_frames;
                scene << "\ttransform_animation: ";
                scene << to_str(key_frames.size()) << ", ";

                for (auto &key_frame : key_frames)
                {
                    write_vec3(&scene, key_frame.transform.position, ", ");
                    write_vec3(&scene, key_frame.transform.rotation, ", ");
                    write_vec3(&scene, key_frame.transform.scale, ", ");

                    scene << to_str(key_frame.duration) << "; ";
                }
                scene << "\n";
            }

            if (ecs.hitpoints.count(id))
            {
                auto &hitpoints = ecs.hitpoints[id];
                scene << "\thitpoints: ";
                scene << to_str(hitpoints) << "; ";
                scene << "\n";
            }

            if (ecs.particle_systems.count(id))
            {
                const auto &particle_sys = ecs.particle_systems[id];

                scene << "\tparticle_system: ";
                if (particle_sys->emitter->type == Emitter::EmitterType::Box)
                {
                    const auto emitter = reinterpret_cast<BoxEmitter *>(particle_sys->emitter.get());

                    scene << "box"
                          << ", ";
                    scene << to_str(particle_sys->emitter->particle_2D) << ", ";
                    write_vec3(&scene, particle_sys->emitter->center, ", ");
                    write_vec3(&scene, emitter->dimensions, ";");
                }

                else if (particle_sys->emitter->type == Emitter::EmitterType::Sphere)
                {
                    const auto emitter = reinterpret_cast<SphereEmitter *>(particle_sys->emitter.get());

                    scene << "sphere"
                          << ", ";
                    scene << to_str(particle_sys->emitter->particle_2D) << ", ";
                    write_vec3(&scene, particle_sys->emitter->center, ", ");
                    scene << to_str(emitter->radius) << ";";
                }

                scene << "\n";
            }

            scene << "}"
                  << "\n\n";
        }

        scene.close();
        LOG_SUCCESS("current scene saved to '%s'", file.c_str());
    }

    void SceneParser::save_config(const str &file)
    {
        std::ofstream scene(file);

        scene << "<post_processing>\n";
        scene << "{"
              << "\n";
        for (const auto &pass : AppConfig::render_passes)
        {
            if (pass.name == "FogPass")
            {
                scene << "\tfog_pass: ";
                scene << to_str(pass.position) << ", ";
                scene << to_str(pass.enabled) << ", ";

                write_vec3(&scene, static_cast<FogPass *>(pass.pass)->fog_color, ", ");
                write_vec2(&scene, static_cast<FogPass *>(pass.pass)->min_max, "; ");
                scene << "\n";
            }

            else if (pass.name == "BloomPass")
            {
                scene << "\tbloom_pass: ";
                scene << to_str(pass.position) << ", ";
                scene << to_str(pass.enabled) << ", ";

                scene << to_str(static_cast<BloomPass *>(pass.pass)->samples) << ", ";
                scene << to_str(static_cast<BloomPass *>(pass.pass)->spread) << ", ";
                scene << to_str(static_cast<BloomPass *>(pass.pass)->threshold) << ", ";
                scene << to_str(static_cast<BloomPass *>(pass.pass)->amount) << ", ";
                scene << "\n";
            }

            else if (pass.name == "SharpenPass")
            {
                scene << "\tsharpen_pass: ";
                scene << to_str(pass.position) << ", ";
                scene << to_str(pass.enabled) << ", ";

                scene << to_str(static_cast<SharpenPass *>(pass.pass)->amount) << "; ";
                scene << "\n";
            }

            else if (pass.name ==
                     "PosterizationPas"
                     "s")
            {
                scene << "\tposterization_pass: ";
                scene << to_str(pass.position) << ", ";
                scene << to_str(pass.enabled) << ", ";

                scene << to_str(static_cast<PosterizationPass *>(pass.pass)->levels) << "; ";
                scene << "\n";
            }

            else if (pass.name ==
                     "PixelizationPas"
                     "s")
            {
                scene << "\tpixelization_pass: ";
                scene << to_str(pass.position) << ", ";
                scene << to_str(pass.enabled) << ", ";

                scene << to_str(static_cast<PixelizationPass *>(pass.pass)->pixel_size) << "; ";
                scene << "\n";
            }

            else if (pass.name == "FXAAPass")
            {
                scene << "\tfxaa_pass: ";
                scene << to_str(pass.position) << ", ";
                scene << to_str(pass.enabled) << "; ";

                scene << "\n";
            }
        }

        scene << "}"
              << "\n\n";

        scene.close();
        LOG_SUCCESS("current configuration saved to '%s'", file.c_str());
    }

    void SceneParser::parse_component(ECS &ecs, const str &line, u32 entity_id, const str &entity_name)
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

            ecs.physics_objects[entity_id] =
                std::make_unique<PhysicsObject>(PhysicsObject(velocity, terminal_velocity, force, stof(mass)));
        }

        else if (component_name == "collider")
        {
            str type, immovable, description_mask, interaction_mask;
            vec3 offset;

            std::getline(iline, type, ',');

            if (type == "sphere")
            {
                str radius;

                std::getline(iline, radius, ',');
                offset = read_vec3(&iline, ',');
                std::getline(iline, immovable, ',');
                std::getline(iline, description_mask, ',');
                std::getline(iline, interaction_mask, ';');

                ecs.colliders[entity_id] = std::make_unique<SphereCollider>(
                    stof(radius), offset, stoi(immovable), std::stoul(description_mask), std::stoul(interaction_mask));
            }

            else if (type == "box")
            {
                str width, height, depth;
                vec3 dimensions = read_vec3(&iline, ',');

                offset = read_vec3(&iline, ',');
                std::getline(iline, immovable, ',');
                std::getline(iline, description_mask, ',');
                std::getline(iline, interaction_mask, ';');

                ecs.colliders[entity_id] = std::make_unique<BoxCollider>(
                    dimensions, offset, stoi(immovable), std::stoul(description_mask), std::stoul(interaction_mask));
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

            ecs.dir_lights[entity_id] =
                std::make_unique<DirectionalLight>(DirectionalLight(ambient, diffuse, specular));
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

            ecs.point_lights[entity_id] = std::make_unique<PointLight>(
                PointLight(ambient, diffuse, specular, stof(constant), stof(linear), stof(quadratic)));
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

            ecs.cameras[entity_id] = std::make_unique<Camera>(
                Camera(offset, world_up, stof(zoom), stof(near), stof(far), stof(lerp_factor)));
        }

        else if (component_name ==
                 "camera_"
                 "controller")
        {
            str sensitivity;
            vec3 speed;

            speed = read_vec3(&iline, ',');
            std::getline(iline, sensitivity, ';');

            ecs.camera_controllers[entity_id] =
                std::make_unique<CameraController>(CameraController(speed, stof(sensitivity)));
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
            ecs.sounds[entity_id][sound_name] =
                std::make_unique<Sound>(Sound(sound_file, sound_name, stof(volume), stoi(play_now), stoi(looping)));
        }

        else if (component_name == "timer")
        {
            str time;
            std::getline(iline, time, ';');
            ecs.timers[entity_id] = std::make_unique<Timer>(Timer(stof(time)));
        }

        else if (component_name ==
                 "transform_"
                 "animation")
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

                Transform transform = {position, rotation, scale};
                key_frames[i] = {transform, stof(duration)};
            }

            ecs.transform_animations[entity_id] = std::make_unique<TransformAnimation>(key_frames);
        }

        else if (component_name == "hitpoints")
        {
            str hitpoint;
            std::getline(iline, hitpoint, ';');

            ecs.hitpoints[entity_id] = std::stof(hitpoint);
        }

        else if (component_name == "particle_system")
        {
            str type, particle_2D;
            vec3 center;

            std::getline(iline, type, ',');
            std::getline(iline, particle_2D, ',');
            center = read_vec3(&iline, ',');

            if (type == "sphere")
            {
                str radius;
                std::getline(iline, radius, ';');

                auto *emitter = new SphereEmitter(center, std::stoi(particle_2D), std::stof(radius));
                ecs.particle_systems[entity_id] = std::make_unique<ParticleSystem>(emitter);
            }

            else if (type == "box")
            {
                vec3 dimensions = read_vec3(&iline, ',');

                auto *emitter = new BoxEmitter(center, std::stoi(particle_2D), dimensions);
                ecs.particle_systems[entity_id] = std::make_unique<ParticleSystem>(emitter);
            }

            else
                LOG_ERROR("invalid particle system type");
        }
    }

    void SceneParser::parse_config(const str &line, const str &config_name)
    {
        if (config_name == "post_processing")
        {
            str parameter;
            str position, enabled;

            std::istringstream iline(line);
            std::getline(iline, parameter, ':');

            auto &post_processing = Game::get().get_renderer().get_post_processing();

            // FogPass
            if (parameter == "fog_pass")
            {
                std::getline(iline, position, ',');
                std::getline(iline, enabled, ',');

                vec3 fog_color = read_vec3(&iline, ',');
                vec2 min_max = read_vec2(&iline, ',');

                auto it = std::find_if(AppConfig::render_passes.begin(),
                                       AppConfig::render_passes.end(),
                                       [](const auto &pass) { return typeid(*pass.pass) == typeid(FogPass); });

                if (it != AppConfig::render_passes.end())
                {
                    auto *fog_pass = static_cast<FogPass *>(it->pass);
                    fog_pass->fog_color = fog_color;
                    fog_pass->min_max = min_max;

                    it->enabled = std::stoul(enabled);
                    it->position = std::stoul(position);
                    post_processing->set_pass(it->id, it->enabled, it->position);
                }
            }

            // BloomPass
            else if (parameter == "bloom_pass")
            {
                std::getline(iline, position, ',');
                std::getline(iline, enabled, ',');

                str samples, spread, threshold, amount;
                std::getline(iline, samples, ',');
                std::getline(iline, spread, ',');
                std::getline(iline, threshold, ',');
                std::getline(iline, amount, ';');

                auto it = std::find_if(AppConfig::render_passes.begin(),
                                       AppConfig::render_passes.end(),
                                       [](const auto &pass) { return typeid(*pass.pass) == typeid(BloomPass); });

                if (it != AppConfig::render_passes.end())
                {
                    auto *bloom_pass = static_cast<BloomPass *>(it->pass);
                    bloom_pass->samples = std::stoul(samples);
                    bloom_pass->spread = std::stof(spread);
                    bloom_pass->threshold = std::stof(threshold);
                    bloom_pass->amount = std::stof(amount);

                    it->enabled = std::stoul(enabled);
                    it->position = std::stoul(position);
                    post_processing->set_pass(it->id, it->enabled, it->position);
                }
            }

            // SharpenPass
            else if (parameter == "sharpen_pass")
            {
                std::getline(iline, position, ',');
                std::getline(iline, enabled, ',');

                str amount;
                std::getline(iline, amount, ';');

                auto it = std::find_if(AppConfig::render_passes.begin(),
                                       AppConfig::render_passes.end(),
                                       [](const auto &pass) { return typeid(*pass.pass) == typeid(SharpenPass); });

                if (it != AppConfig::render_passes.end())
                {
                    auto *sharpen_pass = static_cast<SharpenPass *>(it->pass);
                    sharpen_pass->amount = std::stof(amount);

                    it->enabled = std::stoul(enabled);
                    it->position = std::stoul(position);
                    post_processing->set_pass(it->id, it->enabled, it->position);
                }
            }

            // PosterizationPass
            else if (parameter ==
                     "posterization_"
                     "pass")
            {
                std::getline(iline, position, ',');
                std::getline(iline, enabled, ',');

                str levels;
                std::getline(iline, levels, ';');

                auto it =
                    std::find_if(AppConfig::render_passes.begin(),
                                 AppConfig::render_passes.end(),
                                 [](const auto &pass) { return typeid(*pass.pass) == typeid(PosterizationPass); });

                if (it != AppConfig::render_passes.end())
                {
                    auto *post_pass = static_cast<PosterizationPass *>(it->pass);
                    post_pass->levels = std::stof(levels);

                    it->enabled = std::stoul(enabled);
                    it->position = std::stoul(position);
                    post_processing->set_pass(it->id, it->enabled, it->position);
                }
            }

            // PixelizationPass
            else if (parameter ==
                     "pixelization_"
                     "pass")
            {
                std::getline(iline, position, ',');
                std::getline(iline, enabled, ',');

                str pixel_size;
                std::getline(iline, pixel_size, ';');

                auto it = std::find_if(AppConfig::render_passes.begin(),
                                       AppConfig::render_passes.end(),
                                       [](const auto &pass) { return typeid(*pass.pass) == typeid(PixelizationPass); });

                if (it != AppConfig::render_passes.end())
                {
                    auto *pixel_pass = static_cast<PixelizationPass *>(it->pass);
                    pixel_pass->pixel_size = std::stoul(pixel_size);

                    it->enabled = std::stoul(enabled);
                    it->position = std::stoul(position);
                    post_processing->set_pass(it->id, it->enabled, it->position);
                }
            }

            // FXAAPass
            else if (parameter == "fxaa_pass")
            {
                std::getline(iline, position, ',');
                std::getline(iline, enabled, ';');

                auto it = std::find_if(AppConfig::render_passes.begin(),
                                       AppConfig::render_passes.end(),
                                       [](const auto &pass) { return typeid(*pass.pass) == typeid(PixelizationPass); });

                if (it != AppConfig::render_passes.end())
                {
                    it->enabled = std::stoul(enabled);
                    it->position = std::stoul(position);
                    post_processing->set_pass(it->id, it->enabled, it->position);
                }
            }
        }
    }

    vec3 SceneParser::read_vec3(std::istringstream *iline, char delimiter)
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

    vec2 SceneParser::read_vec2(std::istringstream *iline, char delimiter)
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

    void SceneParser::write_vec3(std::ofstream *scene, const vec3 &vec, const str &end_str)
    {
        *scene << to_str(vec).substr(4) << end_str;
    }

    void SceneParser::write_vec2(std::ofstream *scene, const vec2 &vec, const str &end_str)
    {
        *scene << to_str(vec).substr(4) << end_str;
    }
};  // namespace bls
