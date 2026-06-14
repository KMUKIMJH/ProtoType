#pragma once
#include "../Engine/Input.h"
#include <unordered_map>
#include <string>

namespace CS230
{
    enum class Action
    {
        MoveLeft,
        MoveRight,
        DropThrough,
        Jump,
        Roll,
        Attack,
        Parry,
        SlowMo,
        Interact,
        Restart,
        Count
    };

    class KeyBinding
    {
    public:
        static KeyBinding& GetInstance()
        {
            static KeyBinding instance;
            return instance;
        }

        KeyBinding(const KeyBinding&) = delete;
        KeyBinding& operator=(const KeyBinding&) = delete;

        bool IsActionKeyDown(Action action) const;
        bool IsActionKeyJustPressed(Action action) const;
        bool IsActionKeyJustReleased(Action action) const;

        void SetBinding(Action action, CS230::Input::Keys key);
        void SetBinding(Action action, CS230::Input::MouseButtons button);

        std::string GetBindingName(Action action) const;

    private:
        KeyBinding();

        struct Binding
        {
            bool isMouse;
            CS230::Input::Keys key;
            CS230::Input::MouseButtons mouseBtn;
        };

        std::unordered_map<Action, Binding> bindings;
        std::string KeyToString(CS230::Input::Keys key) const;
    };
}