
#include <GL/glew.h>
#include "TextureManager.h"
#include "../OpenGL/Canvas2D.h"
#include "../OpenGL/Viewport.h"
#include "Engine.h"
#include "Logger.h"
#include "Texture.h"
#include <algorithm>
#include <memory> 
#include <array>

std::shared_ptr<CS230::Texture> CS230::TextureManager::Load(const std::filesystem::path& file_name)
{
    std::filesystem::path key = std::filesystem::weakly_canonical(file_name);
    auto find_it = textures.find(key);
    if (find_it != textures.end())
    {
        return find_it->second;
    }

    std::shared_ptr<Texture> new_texture(new Texture(key));
    textures.emplace(std::move(key), new_texture);

    return new_texture;
}

void CS230::TextureManager::Unload()
{
    textures.clear();
}

namespace
{
    struct RenderToTextureState
    {
        unsigned int framebuffer = 0;
        unsigned int colorAttachment = 0;
        unsigned int prevFramebuffer = 0;
        Math::ivec2                  size{};
        std::array<GLint, 4>         savedViewport{};
        std::array<GLfloat, 4>       savedClearColor{};
    };
    static RenderToTextureState activeState;
}

void CS230::TextureManager::StartRenderTextureMode(int width, int height)
{
    auto& renderer = Engine::GetRenderer2D();

    renderer.EndScene();
    glGetIntegerv(GL_VIEWPORT, activeState.savedViewport.data());
    glGetFloatv(GL_COLOR_CLEAR_VALUE, activeState.savedClearColor.data());
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, reinterpret_cast<GLint*>(&activeState.prevFramebuffer));
    activeState.size = { width, height };
    glGenFramebuffers(1, &activeState.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, activeState.framebuffer);
    glGenTextures(1, &activeState.colorAttachment);
    glBindTexture(GL_TEXTURE_2D, activeState.colorAttachment);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, activeState.colorAttachment, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        Engine::GetLogger().LogError("RenderTexture FBO is not complete");
    }

    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    const double sx = 2.0 / static_cast<double>(width);
    const double sy = -2.0 / static_cast<double>(height);
    Math::TransformationMatrix ndc =
        Math::TranslationMatrix(Math::vec2{ -1.0, 1.0 }) * Math::ScaleMatrix(Math::vec2{ sx, sy });
    renderer.BeginScene(ndc);
}

std::shared_ptr<CS230::Texture> CS230::TextureManager::EndRenderTextureMode()
{
    auto& renderer = Engine::GetRenderer2D();
    renderer.EndScene();
    glBindFramebuffer(GL_FRAMEBUFFER, activeState.prevFramebuffer);
    glViewport(activeState.savedViewport[0], activeState.savedViewport[1], activeState.savedViewport[2], activeState.savedViewport[3]);
    glClearColor(activeState.savedClearColor[0], activeState.savedClearColor[1], activeState.savedClearColor[2], activeState.savedClearColor[3]);
    unsigned int textureHandle = activeState.colorAttachment;

    glDeleteFramebuffers(1, &activeState.framebuffer);
    activeState.framebuffer = 0;
    activeState.prevFramebuffer = 0;

    Math::ivec2 originalSize = { activeState.savedViewport[2], activeState.savedViewport[3] };
    renderer.BeginScene(GAME200::build_viewport_matrix(originalSize));

    auto newTexture = std::shared_ptr<Texture>(new Texture(textureHandle, activeState.size));

    return newTexture;
}