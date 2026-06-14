#include <GL/glew.h>
#include <SDL2/SDL.h>
#include "Window.h"
#include <functional>
#include <sstream>
#include <stdexcept>

namespace
{
    void hint_gl(SDL_GLattr attr, int value)
    {
        SDL_GL_SetAttribute(attr, value);
    }
}

namespace CS230
{
    void Window::Start(std::string_view title)
    {
        setupSDLWindow(title);
        setupOpenGL();

        int winW = 0, winH = 0;
        SDL_GetWindowSize(sdlWindow, &winW, &winH);
        size = { winW, winH };

        int drawableW = 0, drawableH = 0;
        SDL_GL_GetDrawableSize(sdlWindow, &drawableW, &drawableH);
        glViewport(0, 0, drawableW, drawableH);

        const auto bg = GAME200::unpack_color(default_background);
        glClearColor(bg[0], bg[1], bg[2], bg[3]);
    }

    void Window::Update()
    {
        SDL_GL_SwapWindow(sdlWindow);
        SDL_Event event{ 0 };
        while (SDL_PollEvent(&event) != 0)
        {
            if (eventCallback)
            {
                eventCallback(event);
            }

            switch (event.type)
            {
            case SDL_QUIT:
                is_closed = true;
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    is_closed = true;
                }
                else if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED || event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    int winW = 0, winH = 0;
                    SDL_GetWindowSize(sdlWindow, &winW, &winH);
                    size = { winW, winH };

                    int drawableW = 0, drawableH = 0;
                    SDL_GL_GetDrawableSize(sdlWindow, &drawableW, &drawableH);
                    glViewport(0, 0, drawableW, drawableH);
                }
                break;
            }
        }
    }

    bool Window::IsClosed() const
    {
        return is_closed;
    }

    Math::ivec2 Window::GetSize() const
    {
        return { 1920, 1080 };
    }

    Math::ivec2 Window::GetPhysicalSize() const
    {
        return size;
    }

    void Window::Clear(GAME200::RGBA color)
    {
        const auto rgba = GAME200::unpack_color(color);
        glClearColor(rgba[0], rgba[1], rgba[2], rgba[3]);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Window::ForceResize(int desired_width, int desired_height)
    {
        SDL_SetWindowSize(sdlWindow, desired_width, desired_height);
        size = { desired_width, desired_height };
        int drawableW = 0, drawableH = 0;
        SDL_GL_GetDrawableSize(sdlWindow, &drawableW, &drawableH);
        glViewport(0, 0, drawableW, drawableH);
    }

    void Window::SetFullscreen(bool fullscreen, bool borderless)
    {
        Uint32 flags = 0;
        if (fullscreen)
        {
            flags = borderless ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN;
        }
        SDL_SetWindowFullscreen(sdlWindow, flags);
    }

    void Window::SetResolution(int width, int height)
    {
        SDL_SetWindowSize(sdlWindow, width, height);
        SDL_SetWindowPosition(sdlWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        size = { width, height };

        int drawableW = 0, drawableH = 0;
        SDL_GL_GetDrawableSize(sdlWindow, &drawableW, &drawableH);
        glViewport(0, 0, drawableW, drawableH);
    }

    SDL_Window* Window::GetSDLWindow() const
    {
        return sdlWindow;
    }

    SDL_GLContext Window::GetGLContext() const
    {
        return glContext;
    }

    void Window::SetEventCallback(WindowEventCallback callback)
    {
        eventCallback = callback;
    }

    void Window::Shutdown()
    {
        if (!is_cleaned_up)
        {
            SDL_GL_MakeCurrent(sdlWindow, nullptr);
            if (glContext)
            {
                SDL_GL_DeleteContext(glContext);
            }
            if (sdlWindow)
            {
                SDL_DestroyWindow(sdlWindow);
            }
            SDL_Quit();

            is_cleaned_up = true;
        }
    }

    void Window::setupSDLWindow(std::string_view title)
    {
        SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");
        SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "1");

        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            throw std::runtime_error(std::string{ "Failed to init SDL error: " } + SDL_GetError());
        }

#if defined(__EMSCRIPTEN__) 
        hint_gl(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        hint_gl(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        hint_gl(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__) 
        hint_gl(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        hint_gl(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        hint_gl(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        hint_gl(SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG, 1);
#else 
        hint_gl(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        hint_gl(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        hint_gl(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif

        hint_gl(SDL_GL_DOUBLEBUFFER, true);
        hint_gl(SDL_GL_STENCIL_SIZE, 8);
        hint_gl(SDL_GL_DEPTH_SIZE, 24);
        hint_gl(SDL_GL_RED_SIZE, 8);
        hint_gl(SDL_GL_GREEN_SIZE, 8);
        hint_gl(SDL_GL_BLUE_SIZE, 8);
        hint_gl(SDL_GL_ALPHA_SIZE, 8);
        hint_gl(SDL_GL_MULTISAMPLEBUFFERS, 1);
        hint_gl(SDL_GL_MULTISAMPLESAMPLES, 4);

        sdlWindow = SDL_CreateWindow(title.data(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            default_width, default_height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

        if (sdlWindow == nullptr)
        {
            throw std::runtime_error(std::string{ "Failed to create window: " } + SDL_GetError());
        }
    }

    void Window::setupOpenGL()
    {
        glContext = SDL_GL_CreateContext(sdlWindow);
        if (glContext == nullptr)
        {
            throw std::runtime_error(std::string{ "Failed to create opengl context: " } + SDL_GetError());
        }

        SDL_GL_MakeCurrent(sdlWindow, glContext);

#if !defined(__EMSCRIPTEN__)
        glewExperimental = GL_TRUE;
        if (const auto result = glewInit(); GLEW_OK != result)
        {
            std::stringstream ss;
            ss << "Unable to initialize GLEW - error: " << glewGetErrorString(result);
            throw std::runtime_error(ss.str());
        }
#endif
        if (SDL_GL_SetSwapInterval(-1) != 0)
        {
            SDL_GL_SetSwapInterval(1);
        }

        GLint major = 0, minor = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        if (major < 3 || (major == 3 && minor < 3))
        {
            std::stringstream ss;
            ss << "Unsupported OpenGL version " << major << '.' << minor << " - require 3.3 or higher";
            throw std::runtime_error(ss.str());
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
    }

    Window::~Window()
    {
        Shutdown();
    }
}