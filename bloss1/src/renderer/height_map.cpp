#include "renderer/height_map.hpp"

#include "config.hpp"
#include "core/game.hpp"

namespace bls
{
    HeightMap::HeightMap(
        u32 width, u32 height, u32 min_tess_level, u32 max_tess_level, f32 min_distance, f32 max_distance)
    {
        this->min_tess_level = min_tess_level;
        this->max_tess_level = max_tess_level;
        this->min_distance = min_distance;
        this->max_distance = max_distance;

        this->displacement = vec2(0.0f);
        this->displacement_multiplier = vec2(0.0f);
        this->fbm_height = 364.0f;
        this->perlin_height = 64.0f;
        this->fbm_scale = 5.0f;
        this->perlin_scale = 1.0f;
        this->fbm_octaves = 6;
        this->noise_algorithm = 0;

        texture_heights.resize(4);
        texture_layers.resize(4);

        texture_heights[0] = 0.03f;
        texture_heights[1] = 0.62f;
        texture_heights[2] = 0.67f;
        texture_heights[3] = 0.72f;

        this->toggle_gradient = false;

        // Create shaders
        shader = Shader::create("height_map_shader",
                                "bloss1/assets/shaders/height_map.vs",
                                "bloss1/assets/shaders/height_map.fs",
                                "",
                                "bloss1/assets/shaders/height_map.tcs.glsl",
                                "bloss1/assets/shaders/height_map.tes.glsl");

        // Sand -> Trees -> Rock -> Snow
        texture_layers[0] =
            Texture::create("h_t0", "bloss1/assets/textures/coast_sand_01_diff_2k.png", TextureType::Diffuse);
        texture_layers[1] =
            Texture::create("h_t1", "bloss1/assets/textures/coast_sand_rocks_02_diff_2k.png", TextureType::Diffuse);
        texture_layers[2] = Texture::create("h_t2", "bloss1/assets/textures/rock_05_diff_2k.png", TextureType::Diffuse);
        texture_layers[3] = Texture::create("h_t3", "bloss1/assets/textures/snow_02_diff_2k.png", TextureType::Diffuse);

        const f32 w = static_cast<f32>(width);
        const f32 h = static_cast<f32>(height);

        num_vert_per_patch = 4;
        num_patches = 20;

        // Setup vertices
        std::vector<f32> vertices;
        for (u32 i = 0; i < num_patches; i++)
        {
            for (u32 j = 0; j < num_patches; j++)
            {
                vertices.push_back(-w / 2.0f + w * i / static_cast<f32>(num_patches));  // v.x
                vertices.push_back(0.0f);                                               // v.y
                vertices.push_back(-h / 2.0f + h * j / static_cast<f32>(num_patches));  // v.z
                vertices.push_back(i / static_cast<f32>(num_patches));                  // u
                vertices.push_back(j / static_cast<f32>(num_patches));                  // v

                vertices.push_back(-w / 2.0f + w * (i + 1) / static_cast<f32>(num_patches));  // v.x
                vertices.push_back(0.0f);                                                     // v.y
                vertices.push_back(-h / 2.0f + h * j / static_cast<f32>(num_patches));        // v.z
                vertices.push_back((i + 1) / static_cast<f32>(num_patches));                  // u
                vertices.push_back(j / static_cast<f32>(num_patches));                        // v

                vertices.push_back(-w / 2.0f + w * i / static_cast<f32>(num_patches));        // v.x
                vertices.push_back(0.0f);                                                     // v.y
                vertices.push_back(-h / 2.0f + h * (j + 1) / static_cast<f32>(num_patches));  // v.z
                vertices.push_back(i / static_cast<f32>(num_patches));                        // u
                vertices.push_back((j + 1) / static_cast<f32>(num_patches));                  // v

                vertices.push_back(-w / 2.0f + w * (i + 1) / static_cast<f32>(num_patches));  // v.x
                vertices.push_back(0.0f);                                                     // v.y
                vertices.push_back(-h / 2.0f + h * (j + 1) / static_cast<f32>(num_patches));  // v.z
                vertices.push_back((i + 1) / static_cast<f32>(num_patches));                  // u
                vertices.push_back((j + 1) / static_cast<f32>(num_patches));                  // v
            }
        }

        // Setup buffers
        vao = std::unique_ptr<VertexArray>(VertexArray::create());
        vao->bind();

        vbo = std::unique_ptr<VertexBuffer>(
            VertexBuffer::create(static_cast<void*>(vertices.data()), vertices.size() * sizeof(f32)));

        vao->add_vertex_buffer(0, 3, ShaderDataType::Float, false, 5 * sizeof(f32), reinterpret_cast<void*>(0));
        vao->add_vertex_buffer(
            1, 2, ShaderDataType::Float, false, 5 * sizeof(f32), reinterpret_cast<void*>((sizeof(f32) * 3)));

        auto& renderer = Game::get().get_renderer();
        renderer.set_tesselation_patches(num_vert_per_patch);
    }

    HeightMap::~HeightMap()
    {
    }

    void HeightMap::render(const mat4& view, const mat4& projection, f32 dt)
    {
        auto& renderer = Game::get().get_renderer();

        shader->bind();

        shader->set_uniform4("model", mat4(1.0f));
        shader->set_uniform4("view", view);
        shader->set_uniform4("projection", projection);

        displacement += vec2(dt);
        shader->set_uniform2("displacement", displacement * displacement_multiplier);

        shader->set_uniform1("min_tess_level", min_tess_level);
        shader->set_uniform1("max_tess_level", max_tess_level);
        shader->set_uniform1("min_distance", min_distance);
        shader->set_uniform1("max_distance", max_distance);

        shader->set_uniform1("noise_algorithm", noise_algorithm);

        shader->set_uniform1("fbm_scale", fbm_scale);
        shader->set_uniform1("fbm_height", fbm_height);
        shader->set_uniform1("octaves", static_cast<u32>(fbm_octaves));

        shader->set_uniform1("perlin_scale", perlin_scale);
        shader->set_uniform1("perlin_height", perlin_height);

        shader->set_uniform1("toggleGradient", toggle_gradient);
        shader->set_uniform1("layers", static_cast<u32>(texture_heights.size()));
        for (size_t i = 0; i < texture_heights.size(); i++)
        {
            shader->set_uniform1("textures[" + to_str(i) + "]", static_cast<u32>(i) + 4U);
            shader->set_uniform1("heights[" + to_str(i) + "]", texture_heights[i]);
            texture_layers[i]->bind(i + 4U);
        }

        const auto& ecs = Game::get().get_curr_stage().ecs;
        for (auto& [id, dir_light] : ecs->dir_lights)
        {
            auto& direction = ecs->transforms[id]->rotation;
            shader->set_uniform3("dirLight.direction", direction);
            shader->set_uniform3("dirLight.ambient", dir_light->ambient);
            shader->set_uniform3("dirLight.diffuse", dir_light->diffuse);
            shader->set_uniform3("dirLight.specular", dir_light->specular);

            break;
        }

        vao->bind();
        renderer.draw_arrays(RenderingMode::Patches, num_vert_per_patch * num_patches * num_patches);
        vao->unbind();

        // Not exactly right but gives a good estimate
        AppStats::vertices += num_vert_per_patch * num_patches * num_patches;
    }
};  // namespace bls
