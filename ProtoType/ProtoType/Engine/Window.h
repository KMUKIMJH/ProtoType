/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Window.h
Project:    CS230 Engine
Author:     Minchan Cho
Created:    March 30, 2025
*/

#pragma once
#include "../OpenGL/RGBA.h"
#include "Vec2.h"
#include <functional>
#include <gsl/gsl>
#include <string_view>

struct SDL_Window;
typedef void* SDL_GLContext;
typedef union SDL_Event SDL_Event;

namespace CS230
{
    class Window
    {
    public:
        void          Start(std::string_view title);
        void          Update();
        bool          IsClosed() const;
        Math::ivec2   GetSize() const;

        Math::ivec2   GetPhysicalSize() const;

        void          Clear(GAME200::RGBA color);
        void          ForceResize(int desired_width, int desired_height);

        void          SetFullscreen(bool fullscreen, bool borderless = false);
        void          SetResolution(int width, int height);

        SDL_Window* GetSDLWindow() const;
        SDL_GLContext GetGLContext() const;

        using WindowEventCallback = std::function<void(const SDL_Event&)>;
        void SetEventCallback(WindowEventCallback callback);

        Window() = default;
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
        Window(Window&&) noexcept = delete;
        Window& operator=(Window&&) noexcept = delete;

    private:

        void setupSDLWindow(std::string_view title);
        void setupOpenGL();
        void Shutdown();

        gsl::owner<SDL_Window*>         sdlWindow = nullptr;
        gsl::owner<SDL_GLContext>       glContext = nullptr;
        Math::ivec2                     size{ 800, 600 };
        WindowEventCallback             eventCallback;
        bool                      is_closed = false;
        bool                      is_cleaned_up = false;

        static constexpr int default_width = 1920;
        static constexpr int default_height = 1080;
        static constexpr GAME200::RGBA default_background{ GAME200::WHITE };
    };
}