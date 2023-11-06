#include "renderer/height_map.hpp"

namespace bls
{
    HeightMap::HeightMap(const str& texture_path) : texture_path(texture_path)
    {
        // Create shaders
        shader = Shader::create(
            "height_map_shader", "bloss1/assets/shaders/height_map.vs", "bloss1/assets/shaders/height_map.fs");

        // Load heightmap texture
        stbi_set_flip_vertically_on_load(true);

        unsigned char* data = stbi_load(texture_path.c_str(), &width, &height, &channels, 0);

        if (data)
        {
            LOG_INFO("Loaded heightmap of size %d x %d", width, height);
        }

        else
            throw std::runtime_error("failed to load height map");

        // Setup vertices
        std::vector<f32> vertices;
        f32 scale_y = 64.0f / 256.0f, shift_y = 16.0f;
        i32 rez = 1;
        for (i32 i = 0; i < height; i++)
        {
            for (i32 j = 0; j < width; j++)
            {
                unsigned char* pixelOffset = data + (j + width * i) * channels;
                unsigned char y = pixelOffset[0];

                // vertex
                vertices.push_back(-height / 2.0f + height * i / static_cast<f32>(height));  // vx
                vertices.push_back(static_cast<i32>(y) * scale_y - shift_y);                 // vy
                vertices.push_back(-width / 2.0f + width * j / static_cast<f32>(width));     // vz
            }
        }

        stbi_image_free(data);

        num_strips = (height - 1) / rez;
        num_triangles_per_strip = (width / rez) * 2 - 2;

        // Setup indices
        std::vector<u32> indices;
        for (i32 i = 0; i < height - 1; i += rez)
            for (i32 j = 0; j < width; j += rez)
                for (i32 k = 0; k < 2; k++) indices.push_back(j + width * (i + k * rez));

        // Setup buffers
        vao = std::unique_ptr<VertexArray>(VertexArray::create());
        vao->bind();

        vbo = std::unique_ptr<VertexBuffer>(
            VertexBuffer::create(static_cast<void*>(vertices.data()), vertices.size() * sizeof(f32)));
        vao->add_vertex_buffer(0, 3, ShaderDataType::Float, false, 0, (void*)0);

        ebo = std::unique_ptr<IndexBuffer>(IndexBuffer::create(indices, indices.size()));
    }

    HeightMap::~HeightMap()
    {
    }

    void HeightMap::render(const mat4& view, const mat4& projection)
    {
        auto& renderer = Game::get().get_renderer();

        shader->bind();

        shader->set_uniform4("projection", projection);
        shader->set_uniform4("view", view);
        shader->set_uniform4("model", mat4(1.0f));

        vao->bind();

        renderer.set_face_culling(false);
        for (u32 strip = 0; strip < num_strips; strip++)
        {
            renderer.draw_indexed(RenderingMode::TriangleStrip,
                                  num_triangles_per_strip + 2,
                                  reinterpret_cast<void*>(sizeof(u32) * (num_triangles_per_strip + 2) * strip));
        }
        renderer.set_face_culling(true);

        AppStats::vertices += num_strips * num_triangles_per_strip * 3;
    }
};  // namespace bls
