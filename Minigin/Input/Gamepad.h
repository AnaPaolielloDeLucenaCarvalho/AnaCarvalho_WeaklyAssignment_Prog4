#ifndef GAMEPAD_H
#define GAMEPAD_H

#include <memory>


// DESIGN PATTERN - Pimpl Idiom (Bridge)
// I hid the messy XInput Windows-specific code inside a private implementation class. This keeps the header completely clean and ensures no Windows headers leak into the rest of the engine.

namespace dae
{
    class Gamepad final
    {
    public:
        enum class ControllerButton
        {
            DPadUp = 0x0001,
            DPadDown = 0x0002,
            DPadLeft = 0x0004,
            DPadRight = 0x0008,
            ButtonStart = 0x0010,
            LeftShoulder = 0x0100,
            RightShoulder = 0x0200,
            ButtonA = 0x1000,
            ButtonB = 0x2000,
            ButtonX = 0x4000,
            ButtonY = 0x8000,
            RightTrigger = 0x40000,
            LeftTrigger = 0x80000
        };

        explicit Gamepad(unsigned int controllerIndex);
        ~Gamepad();

        Gamepad(const Gamepad&) = delete;
        Gamepad(Gamepad&&) = delete;
        Gamepad& operator=(const Gamepad&) = delete;
        Gamepad& operator=(Gamepad&&) = delete;

        void Update();

        bool IsDown(ControllerButton button) const;
        bool IsUp(ControllerButton button) const;
        bool IsPressed(ControllerButton button) const;

    private:
        class GamepadImpl;
        std::unique_ptr<GamepadImpl> m_pImpl;
    };
}

#endif