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
        GamepadImpl(unsigned int index)
            : m_ControllerIndex(index)
            , m_ButtonsPressedThisFrame(0)
            , m_ButtonsReleasedThisFrame(0)
        {
            ZeroMemory(&m_CurrentState, sizeof(XINPUT_STATE));
            ZeroMemory(&m_PreviousState, sizeof(XINPUT_STATE));
        }

        void Update()
        {
            m_PreviousState = m_CurrentState;
            ZeroMemory(&m_CurrentState, sizeof(XINPUT_STATE));
            XInputGetState(m_ControllerIndex, &m_CurrentState);

            auto buttonChanges = m_CurrentState.Gamepad.wButtons ^ m_PreviousState.Gamepad.wButtons;
            m_ButtonsPressedThisFrame = buttonChanges & m_CurrentState.Gamepad.wButtons;
            m_ButtonsReleasedThisFrame = buttonChanges & (~m_CurrentState.Gamepad.wButtons);
        }

        bool IsDown(ControllerButton button) const { return m_ButtonsPressedThisFrame & static_cast<unsigned int>(button); }
        bool IsUp(ControllerButton button) const { return m_ButtonsReleasedThisFrame & static_cast<unsigned int>(button); }
        bool IsPressed(ControllerButton button) const { return m_CurrentState.Gamepad.wButtons & static_cast<unsigned int>(button); }

    private:
        unsigned int m_ControllerIndex;
        XINPUT_STATE m_PreviousState;
        XINPUT_STATE m_CurrentState;
        int m_ButtonsPressedThisFrame;
        int m_ButtonsReleasedThisFrame;
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
            : m_ControllerIndex(index)
            , m_pGamepad(nullptr)
            , m_CurrentButtons(0)
            , m_PreviousButtons(0)
        {
            // SDL3 uses instance IDs, but we can open by index for simplicity here
            m_pGamepad = SDL_OpenGamepad(index);
        }

        ~GamepadImpl()
        {
            if (m_pGamepad) SDL_CloseGamepad(m_pGamepad);
        }

        void Update()
        {
            if (!m_pGamepad) return;

            m_PreviousButtons = m_CurrentButtons;
            m_CurrentButtons = 0;

            // Map your enum to SDL buttons and build a bitmask
            auto MapButton = [&](ControllerButton daeButton, SDL_GamepadButton sdlButton) {
                if (SDL_GetGamepadButton(m_pGamepad, sdlButton)) {
                    m_CurrentButtons |= static_cast<unsigned int>(daeButton);
                }
                };

            MapButton(ControllerButton::DPadUp, SDL_GAMEPAD_BUTTON_DPAD_UP);
            MapButton(ControllerButton::DPadDown, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
            MapButton(ControllerButton::DPadLeft, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
            MapButton(ControllerButton::DPadRight, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
            MapButton(ControllerButton::ButtonA, SDL_GAMEPAD_BUTTON_SOUTH);
            MapButton(ControllerButton::ButtonB, SDL_GAMEPAD_BUTTON_EAST);
        }

        bool IsDown(ControllerButton button) const
        {
            unsigned int bit = static_cast<unsigned int>(button);
            return (m_CurrentButtons & bit) && !(m_PreviousButtons & bit);
        }

        bool IsUp(ControllerButton button) const
        {
            unsigned int bit = static_cast<unsigned int>(button);
            return !(m_CurrentButtons & bit) && (m_PreviousButtons & bit);
        }

        bool IsPressed(ControllerButton button) const
        {
            return m_CurrentButtons & static_cast<unsigned int>(button);
        }

    private:
        unsigned int m_ControllerIndex;
        SDL_Gamepad* m_pGamepad;
        unsigned int m_CurrentButtons;
        unsigned int m_PreviousButtons;
    };
}
#endif