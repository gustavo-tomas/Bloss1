#include "renderer/post_processing.hpp"
#include "core/game.hpp"

namespace bls
{
    PostProcessingTexture::PostProcessingTexture(u32 width, u32 height)
    {
        this->width = width;
        this->height = height;

        // Create a fbo and a color attachment texture to render the scene
        fbo = std::unique_ptr<FrameBuffer>(FrameBuffer::create());
        fbo_texture = Texture::create(width, height,
                                      ImageFormat::RGB8,
                                      TextureParameter::Linear, TextureParameter::Linear,
                                      TextureParameter::Linear, TextureParameter::Linear);
        fbo->attach_texture(fbo_texture.get());
        rbo_depth = std::unique_ptr<RenderBuffer>(RenderBuffer::create(width, height, AttachmentType::Depth));

        fbo->check();
        fbo->unbind();

        quad = std::make_unique<Quad>(Game::get().get_renderer());
        shader = Shader::create("post_processing", "bloss1/assets/shaders/post/base.vs", "bloss1/assets/shaders/post/identity.fs");
        shader->bind();
        shader->set_uniform1("fbo_texture", 0U);
    }

    PostProcessingTexture::~PostProcessingTexture()
    {
        std::cout << "post processing texture destroyed successfully\n";
    }

    void PostProcessingTexture::bind()
    {
        auto& renderer = Game::get().get_renderer();

        fbo->bind();

        renderer.clear();
        renderer.clear_color(vec4(0.0f, 0.0f, 0.0f, 1.0f));
    }

    void PostProcessingTexture::unbind()
    {
        fbo->unbind();
    }

    void PostProcessingTexture::render()
    {
        shader->bind();
        fbo_texture->bind(0);
        quad->render();
    }
};
