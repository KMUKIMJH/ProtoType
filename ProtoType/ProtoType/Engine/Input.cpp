/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Input.cpp
Project:    CS230 Engine
Author:     Minchan Cho
Created:    March 16, 2023
*/

#include "Input.h"
#include "Engine.h"
#include <SDL2/SDL.h>

constexpr CS230::Input::Keys& operator++(CS230::Input::Keys& key) noexcept
{
    key = static_cast<CS230::Input::Keys>(static_cast<unsigned>(key) + 1);
    return key;
}

// Provide readable names for keys for debug logging
static const char* cs230_key_name(CS230::Input::Keys k) noexcept
{
    using K = CS230::Input::Keys;
    switch (k) {
    case K::None: return "None";
    case K::A: return "A"; case K::B: return "B"; case K::C: return "C"; case K::D: return "D"; case K::E: return "E";
    case K::F: return "F"; case K::G: return "G"; case K::H: return "H"; case K::I: return "I"; case K::J: return "J";
    case K::K: return "K"; case K::L: return "L"; case K::M: return "M"; case K::N: return "N"; case K::O: return "O";
    case K::P: return "P"; case K::Q: return "Q"; case K::R: return "R"; case K::S: return "S"; case K::T: return "T";
    case K::U: return "U"; case K::V: return "V"; case K::W: return "W"; case K::X: return "X"; case K::Y: return "Y";
    case K::Z: return "Z";
    case K::Space: return "Space"; case K::Enter: return "Enter"; case K::Left: return "Left"; case K::Up: return "Up";
    case K::Right: return "Right"; case K::Down: return "Down"; case K::Escape: return "Escape"; case K::Tab: return "Tab";
    case K::LShift: return "LShift"; case K::RShift: return "RShift"; case K::PageUp: return "PageUp"; case K::PageDown: return "PageDown";
    case K::KP_0: return "KP_0"; case K::KP_1: return "KP_1"; case K::KP_2: return "KP_2"; case K::KP_3: return "KP_3";
    case K::KP_4: return "KP_4"; case K::KP_5: return "KP_5"; case K::KP_6: return "KP_6"; case K::KP_7: return "KP_7";
    case K::KP_8: return "KP_8"; case K::KP_9: return "KP_9";
    case K::Digit0: return "0"; case K::Digit1: return "1"; case K::Digit2: return "2"; case K::Digit3: return "3";
    case K::Digit4: return "4"; case K::Digit5: return "5"; case K::Digit6: return "6"; case K::Digit7: return "7";
    case K::Digit8: return "8"; case K::Digit9: return "9";
    default: return "Unknown";
    }
}

static constexpr int cs230_to_sdl_scancode(CS230::Input::Keys cs230_key) noexcept
{
    using K = CS230::Input::Keys;
    switch (cs230_key) {
    case K::A: return SDL_SCANCODE_A;
    case K::B: return SDL_SCANCODE_B;
    case K::C: return SDL_SCANCODE_C;
    case K::D: return SDL_SCANCODE_D;
    case K::E: return SDL_SCANCODE_E;
    case K::F: return SDL_SCANCODE_F;
    case K::G: return SDL_SCANCODE_G;
    case K::H: return SDL_SCANCODE_H;
    case K::I: return SDL_SCANCODE_I;
    case K::J: return SDL_SCANCODE_J;
    case K::K: return SDL_SCANCODE_K;
    case K::L: return SDL_SCANCODE_L;
    case K::M: return SDL_SCANCODE_M;
    case K::N: return SDL_SCANCODE_N;
    case K::O: return SDL_SCANCODE_O;
    case K::P: return SDL_SCANCODE_P;
    case K::Q: return SDL_SCANCODE_Q;
    case K::R: return SDL_SCANCODE_R;
    case K::S: return SDL_SCANCODE_S;
    case K::T: return SDL_SCANCODE_T;
    case K::U: return SDL_SCANCODE_U;
    case K::V: return SDL_SCANCODE_V;
    case K::W: return SDL_SCANCODE_W;
    case K::X: return SDL_SCANCODE_X;
    case K::Y: return SDL_SCANCODE_Y;
    case K::Z: return SDL_SCANCODE_Z;
    case K::Space: return SDL_SCANCODE_SPACE;
    case K::Enter: return SDL_SCANCODE_RETURN;
    case K::Left: return SDL_SCANCODE_LEFT;
    case K::Up: return SDL_SCANCODE_UP;
    case K::Right: return SDL_SCANCODE_RIGHT;
    case K::Down: return SDL_SCANCODE_DOWN;
    case K::Escape: return SDL_SCANCODE_ESCAPE;
    case K::Tab: return SDL_SCANCODE_TAB;
    case K::LShift: return SDL_SCANCODE_LSHIFT;
    case K::RShift: return SDL_SCANCODE_RSHIFT;
    case K::PageUp: return SDL_SCANCODE_PAGEUP;
    case K::PageDown: return SDL_SCANCODE_PAGEDOWN;
    case K::KP_0: return SDL_SCANCODE_KP_0;
    case K::KP_1: return SDL_SCANCODE_KP_1;
    case K::KP_2: return SDL_SCANCODE_KP_2;
    case K::KP_3: return SDL_SCANCODE_KP_3;
    case K::KP_4: return SDL_SCANCODE_KP_4;
    case K::KP_5: return SDL_SCANCODE_KP_5;
    case K::KP_6: return SDL_SCANCODE_KP_6;
    case K::KP_7: return SDL_SCANCODE_KP_7;
    case K::KP_8: return SDL_SCANCODE_KP_8;
    case K::KP_9: return SDL_SCANCODE_KP_9;
    case K::Digit0: return SDL_SCANCODE_0;
    case K::Digit1: return SDL_SCANCODE_1;
    case K::Digit2: return SDL_SCANCODE_2;
    case K::Digit3: return SDL_SCANCODE_3;
    case K::Digit4: return SDL_SCANCODE_4;
    case K::Digit5: return SDL_SCANCODE_5;
    case K::Digit6: return SDL_SCANCODE_6;
    case K::Digit7: return SDL_SCANCODE_7;
    case K::Digit8: return SDL_SCANCODE_8;
    case K::Digit9: return SDL_SCANCODE_9;
    default: return -1;
    }
}

CS230::Input::Input() {
    keys_down.resize(static_cast<int>(Keys::Count));
    previous_keys_down.resize(static_cast<int>(Keys::Count));
    mouse_down.resize(static_cast<int>(MouseButtons::Count));
    previous_mouse_down.resize(static_cast<int>(MouseButtons::Count));
}

void CS230::Input::SetKeyDown(Keys key, bool value)
{
    keys_down[static_cast<int>(key)] = value;
}

void CS230::Input::Update()
{
    previous_keys_down = keys_down;
    previous_mouse_down = mouse_down;

    int len = 0;
    const Uint8* state = SDL_GetKeyboardState(&len);

    for (Keys key = Keys::A; key < Keys::Count; ++key)
    {
        const int sc = cs230_to_sdl_scancode(key);
        if (sc >= 0 && sc < len)
        {
            const bool down = state[sc] != 0;
            SetKeyDown(key, down);
            if (KeyJustPressed(key))
            {
                Engine::GetLogger().LogDebug(std::string("Key Pressed: ") + cs230_key_name(key));
            }
            else if (KeyJustReleased(key))
            {
                Engine::GetLogger().LogDebug(std::string("Key Released: ") + cs230_key_name(key));
            }
        }
    }

    int x = 0, y = 0;
    Uint32 mstate = SDL_GetMouseState(&x, &y);
    mouse_down[static_cast<int>(MouseButtons::Left)] = (mstate & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    mouse_down[static_cast<int>(MouseButtons::Right)] = (mstate & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
    mouse_down[static_cast<int>(MouseButtons::Middle)] = (mstate & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
}

bool CS230::Input::KeyDown(Keys key)
{
    return keys_down[static_cast<int>(key)];
}

bool CS230::Input::KeyJustPressed(Keys key)
{
    return keys_down[static_cast<int>(key)] == true && previous_keys_down[static_cast<int>(key)] == false;
}

bool CS230::Input::KeyJustReleased(Keys key)
{
    return keys_down[static_cast<int>(key)] == false && previous_keys_down[static_cast<int>(key)] == true;
}

bool CS230::Input::MouseDown(MouseButtons button)
{
    return mouse_down[static_cast<int>(button)];
}

bool CS230::Input::MouseJustPressed(MouseButtons button)
{
    return mouse_down[static_cast<int>(button)] == true && previous_mouse_down[static_cast<int>(button)] == false;
}

bool CS230::Input::MouseJustReleased(MouseButtons button)
{
    return mouse_down[static_cast<int>(button)] == false && previous_mouse_down[static_cast<int>(button)] == true;
}


