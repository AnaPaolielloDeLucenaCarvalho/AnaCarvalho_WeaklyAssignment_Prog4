#ifndef GAMEPAD_H
#define GAMEPAD_H

#include <memory>

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
            ButtonA = 0x1000,
            ButtonB = 0x2000
        };

        explicit Gamepad(unsigned int controllerIndex);
        ~Gamepad();

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