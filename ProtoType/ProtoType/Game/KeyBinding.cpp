#include "KeyBinding.h"
#include "../Engine/Engine.h"

namespace CS230
{
    KeyBinding::KeyBinding()
    {
        bindings[Action::MoveLeft] = { false, CS230::Input::Keys::A, CS230::Input::MouseButtons::Left };
        bindings[Action::MoveRight] = { false, CS230::Input::Keys::D, CS230::Input::MouseButtons::Left };
        bindings[Action::DropThrough] = { false, CS230::Input::Keys::S, CS230::Input::MouseButtons::Left };
        bindings[Action::Jump] = { false, CS230::Input::Keys::Space, CS230::Input::MouseButtons::Left };
        bindings[Action::Roll] = { false, CS230::Input::Keys::LShift, CS230::Input::MouseButtons::Left };
        bindings[Action::Attack] = { true, CS230::Input::Keys::None, CS230::Input::MouseButtons::Left };
        bindings[Action::Parry] = { true, CS230::Input::Keys::None, CS230::Input::MouseButtons::Right };
        bindings[Action::SlowMo] = { false, CS230::Input::Keys::Q, CS230::Input::MouseButtons::Left };
        bindings[Action::Interact] = { false, CS230::Input::Keys::E, CS230::Input::MouseButtons::Left };
        bindings[Action::Restart] = { false, CS230::Input::Keys::R, CS230::Input::MouseButtons::Left };
    }

    bool KeyBinding::IsActionKeyDown(Action action) const
    {
        auto it = bindings.find(action);
        if (it != bindings.end())
        {
            if (it->second.isMouse)
            {
                return Engine::GetInput().MouseDown(it->second.mouseBtn);
            }
            else
            {
                return Engine::GetInput().KeyDown(it->second.key);
            }
        }
        return false;
    }

    bool KeyBinding::IsActionKeyJustPressed(Action action) const
    {
        auto it = bindings.find(action);
        if (it != bindings.end())
        {
            if (it->second.isMouse)
            {
                return Engine::GetInput().MouseJustPressed(it->second.mouseBtn);
            }
            else
            {
                return Engine::GetInput().KeyJustPressed(it->second.key);
            }
        }
        return false;
    }

    bool KeyBinding::IsActionKeyJustReleased(Action action) const
    {
        auto it = bindings.find(action);
        if (it != bindings.end())
        {
            if (it->second.isMouse)
            {
                return Engine::GetInput().MouseJustReleased(it->second.mouseBtn);
            }
            else
            {
                return Engine::GetInput().KeyJustReleased(it->second.key);
            }
        }
        return false;
    }

    void KeyBinding::SetBinding(Action action, CS230::Input::Keys key)
    {
        bindings[action] = { false, key, CS230::Input::MouseButtons::Left };
    }

    void KeyBinding::SetBinding(Action action, CS230::Input::MouseButtons button)
    {
        bindings[action] = { true, CS230::Input::Keys::None, button };
    }

    std::string KeyBinding::GetBindingName(Action action) const
    {
        auto it = bindings.find(action);
        if (it != bindings.end())
        {
            if (it->second.isMouse)
            {
                if (it->second.mouseBtn == CS230::Input::MouseButtons::Left)
                {
                    return "L-Click";
                }
                if (it->second.mouseBtn == CS230::Input::MouseButtons::Right)
                {
                    return "R-Click";
                }
                if (it->second.mouseBtn == CS230::Input::MouseButtons::Middle)
                {
                    return "M-Click";
                }
            }
            else
            {
                return KeyToString(it->second.key);
            }
        }
        return "Unbound";
    }

    std::string KeyBinding::KeyToString(CS230::Input::Keys key) const
    {
        switch (key)
        {
        case CS230::Input::Keys::A:
        {
            return "A";
        }
        case CS230::Input::Keys::D:
        {
            return "D";
        }
        case CS230::Input::Keys::S:
        {
            return "S";
        }
        case CS230::Input::Keys::W:
        {
            return "W";
        }
        case CS230::Input::Keys::E:
        {
            return "E";
        }
        case CS230::Input::Keys::Q:
        {
            return "Q";
        }
        case CS230::Input::Keys::R:
        {
            return "R";
        }
        case CS230::Input::Keys::F:
        {
            return "F";
        }
        case CS230::Input::Keys::Tab:
        {
            return "Tab";
        }
        case CS230::Input::Keys::Space:
        {
            return "Space";
        }
        case CS230::Input::Keys::LShift:
        {
            return "LShift";
        }
        case CS230::Input::Keys::RShift:
        {
            return "RShift";
        }
        case CS230::Input::Keys::Enter:
        {
            return "Enter";
        }
        case CS230::Input::Keys::Escape:
        {
            return "Esc";
        }
        case CS230::Input::Keys::Up:
        {
            return "Up";
        }
        case CS230::Input::Keys::Down:
        {
            return "Down";
        }
        case CS230::Input::Keys::Left:
        {
            return "Left";
        }
        case CS230::Input::Keys::Right:
        {
            return "Right";
        }
        default:
        {
            return "Key";
        }
        }
    }
}