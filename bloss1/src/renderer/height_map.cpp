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

        // Create shaders
        shader = Shader::create("height_map_shader",
                                "bloss1/assets/shaders/height_map.vs",
                                "bloss1/assets/shaders/height_map.fs",
                                "",
                                "bloss1/assets/shaders/height_map.tcs.glsl",
                                "bloss1/assets/shaders/height_map.tes.glsl");

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

    void HeightMap::render(const mat4& view, const mat4& projection)
    {
        auto& renderer = Game::get().get_renderer();

        shader->bind();

        shader->set_uniform4("model", mat4(1.0f));
        shader->set_uniform4("view", view);
        shader->set_uniform4("projection", projection);

        shader->set_uniform1("min_tess_level", min_tess_level);
        shader->set_uniform1("max_tess_level", max_tess_level);
        shader->set_uniform1("min_distance", min_distance);
        shader->set_uniform1("max_distance", max_distance);

        vao->bind();
        renderer.draw_arrays(RenderingMode::Patches, num_vert_per_patch * num_patches * num_patches);
        vao->unbind();

        // Not exactly right but gives a good estimate
        AppStats::vertices += num_vert_per_patch * num_patches * num_patches;
    }
};  // namespace bls
