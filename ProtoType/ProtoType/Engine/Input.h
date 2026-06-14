/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Input.h
Project:    CS230 Engine
Author:     Minchan Cho
Created:    March 16, 2023
*/

#pragma once
#include <vector>

namespace CS230 {
    class Input {
    public:
        enum class Keys
        {
            None,
            A,
            B,
            C,
            D,
            E,
            F,
            G,
            H,
            I,
            J,
            K,
            L,
            M,
            N,
            O,
            P,
            Q,
            R,
            S,
            T,
            U,
            V,
            W,
            X,
            Y,
            Z,
            Space,
            Enter,
            Left,
            Up,
            Right,
            Down,
            Escape,
            Tab,
            LShift,
            RShift,
            PageUp,
            PageDown,
            KP_0,
            KP_1,
            KP_2,
            KP_3,
            KP_4,
            KP_5,
            KP_6,
            KP_7,
            KP_8,
            KP_9,
            Digit0,
            Digit1,
            Digit2,
            Digit3,
            Digit4,
            Digit5,
            Digit6,
            Digit7,
            Digit8,
            Digit9,
            Count
        };

        enum class MouseButtons {
            Left = 0,
            Right = 1,
            Middle = 2,
            Count
        };


        Input();
        void Update();
        void SetKeyDown(Keys key, bool value);
        bool KeyDown(Keys key);
        bool KeyJustPressed(Keys key);
        bool KeyJustReleased(Keys key);

        bool MouseDown(MouseButtons button);
        bool MouseJustPressed(MouseButtons button);
        bool MouseJustReleased(MouseButtons button);

    private:
        std::vector<bool> keys_down;
        std::vector<bool> previous_keys_down;

        std::vector<bool> mouse_down;
        std::vector<bool> previous_mouse_down;

    };

}
