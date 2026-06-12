#include "Gamepad.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Xinput.h>
#pragma comment(lib, "xinput.lib")

namespace dae
{
    class Gamepad::GamepadImpl
    {
    public:
        GamepadImpl(unsigned int index) : m_controllerIndex(index)
        {
            ZeroMemory(&m_currentState, sizeof(XINPUT_STATE));
            ZeroMemory(&m_previousState, sizeof(XINPUT_STATE));
        }

        void Update()
        {
            m_previousState = m_currentState;
            ZeroMemory(&m_currentState, sizeof(XINPUT_STATE));
            XInputGetState(m_controllerIndex, &m_currentState);

            unsigned int currentButtons = m_currentState.Gamepad.wButtons;
            if (m_currentState.Gamepad.bRightTrigger > 153)
                currentButtons |= static_cast<unsigned int>(ControllerButton::RightTrigger);
            if (m_currentState.Gamepad.bLeftTrigger > 153)
                currentButtons |= static_cast<unsigned int>(ControllerButton::LeftTrigger);

            unsigned int previousButtons = m_previousState.Gamepad.wButtons;
            if (m_previousState.Gamepad.bRightTrigger > 153)
                previousButtons |= static_cast<unsigned int>(ControllerButton::RightTrigger);
            if (m_previousState.Gamepad.bLeftTrigger > 153)
                previousButtons |= static_cast<unsigned int>(ControllerButton::LeftTrigger);

            auto buttonChanges = currentButtons ^ previousButtons;
            m_buttonsPressedThisFrame = buttonChanges & currentButtons;
            m_buttonsReleasedThisFrame = buttonChanges & (~currentButtons);
            m_currentButtons = currentButtons;
        }

        bool IsDown(ControllerButton button) const { return m_buttonsPressedThisFrame & static_cast<unsigned int>(button); }
        bool IsUp(ControllerButton button) const { return m_buttonsReleasedThisFrame & static_cast<unsigned int>(button); }
        bool IsPressed(ControllerButton button) const { return m_currentButtons & static_cast<unsigned int>(button); }

    private:
        unsigned int m_controllerIndex;
        XINPUT_STATE m_previousState;
        XINPUT_STATE m_currentState;
        unsigned int m_currentButtons{ 0 };
        unsigned int m_buttonsPressedThisFrame{ 0 };
        unsigned int m_buttonsReleasedThisFrame{ 0 };
    };
}

#else 
#include <SDL3/SDL.h>

namespace dae
{
    class Gamepad::GamepadImpl
    {
    public:
        GamepadImpl(unsigned int index)
            : m_controllerIndex(index)
            , m_pGamepad(nullptr)
            , m_currentButtons(0)
            , m_previousButtons(0)
        {
            m_pGamepad = SDL_OpenGamepad(m_controllerIndex);
        }

        ~GamepadImpl() { if (m_pGamepad) SDL_CloseGamepad(m_pGamepad); }

        void Update()
        {
            if (!m_pGamepad)
            {
                int count = 0;
                SDL_JoystickID* gamepads = SDL_GetGamepads(&count);

                if (count > 0)
                {
                    m_pGamepad = SDL_OpenGamepad(gamepads[0]);

                    if (m_pGamepad) {
                        SDL_Log("SUCCESS: Opened Gamepad with ID: %u", (unsigned int)gamepads[0]);
                    }
                }
                SDL_free(gamepads);
            }

            if (!m_pGamepad) return;

            m_previousButtons = m_currentButtons;
            m_currentButtons = 0;

            auto Map = [&](ControllerButton daeB, SDL_GamepadButton sdlB) {
                if (SDL_GetGamepadButton(m_pGamepad, sdlB))
                    m_currentButtons |= static_cast<unsigned int>(daeB);
                };

            Map(ControllerButton::DPadUp, SDL_GAMEPAD_BUTTON_DPAD_UP);
            Map(ControllerButton::DPadDown, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
            Map(ControllerButton::DPadLeft, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
            Map(ControllerButton::DPadRight, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
            Map(ControllerButton::ButtonStart, SDL_GAMEPAD_BUTTON_START);
            Map(ControllerButton::LeftShoulder, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
            Map(ControllerButton::RightShoulder, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
            Map(ControllerButton::ButtonA, SDL_GAMEPAD_BUTTON_SOUTH);
            Map(ControllerButton::ButtonB, SDL_GAMEPAD_BUTTON_EAST);
            Map(ControllerButton::ButtonY, SDL_GAMEPAD_BUTTON_NORTH);

            if (SDL_GetGamepadAxis(m_pGamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > 19660)
                m_currentButtons |= static_cast<unsigned int>(ControllerButton::RightTrigger);
            if (SDL_GetGamepadAxis(m_pGamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER) > 19660)
                m_currentButtons |= static_cast<unsigned int>(ControllerButton::LeftTrigger);
        }

        bool IsDown(ControllerButton b) const {
            return (m_currentButtons & static_cast<unsigned int>(b)) && !(m_previousButtons & static_cast<unsigned int>(b));
        }
        bool IsUp(ControllerButton b) const {
            return !(m_currentButtons & static_cast<unsigned int>(b)) && (m_previousButtons & static_cast<unsigned int>(b));
        }
        bool IsPressed(ControllerButton b) const {
            return m_currentButtons & static_cast<unsigned int>(b);
        }

    private:
        unsigned int m_controllerIndex;
        SDL_Gamepad* m_pGamepad;
        unsigned int m_currentButtons;
        unsigned int m_previousButtons;
    };
}
#endif

namespace dae
{
    Gamepad::Gamepad(unsigned int index) : m_pImpl(std::make_unique<GamepadImpl>(index)) {}
    Gamepad::~Gamepad() = default;
    void Gamepad::Update() { m_pImpl->Update(); }
    bool Gamepad::IsDown(ControllerButton button) const { return m_pImpl->IsDown(button); }
    bool Gamepad::IsUp(ControllerButton button) const { return m_pImpl->IsUp(button); }
    bool Gamepad::IsPressed(ControllerButton button) const { return m_pImpl->IsPressed(button); }
}