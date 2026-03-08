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
        GamepadImpl(unsigned int index) : m_ControllerIndex(index)
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
        int m_ButtonsPressedThisFrame{ 0 };
        int m_ButtonsReleasedThisFrame{ 0 };
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
                    if (m_pGamepad) SDL_Log("SUCCESS: Gamepad opened in Web Build!");
                }
                SDL_free(gamepads);
            }

            if (!m_pGamepad) return;

            m_PreviousButtons = m_CurrentButtons;
            m_CurrentButtons = 0;

            auto Map = [&](ControllerButton daeB, SDL_GamepadButton sdlB) {
                if (SDL_GetGamepadButton(m_pGamepad, sdlB))
                    m_CurrentButtons |= static_cast<unsigned int>(daeB);
                };

            Map(ControllerButton::DPadUp, SDL_GAMEPAD_BUTTON_DPAD_UP);
            Map(ControllerButton::DPadDown, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
            Map(ControllerButton::DPadLeft, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
            Map(ControllerButton::DPadRight, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
            Map(ControllerButton::ButtonA, SDL_GAMEPAD_BUTTON_SOUTH);
            Map(ControllerButton::ButtonB, SDL_GAMEPAD_BUTTON_EAST);
        }

        bool IsDown(ControllerButton b) const {
            return (m_CurrentButtons & static_cast<unsigned int>(b)) && !(m_PreviousButtons & static_cast<unsigned int>(b));
        }
        bool IsUp(ControllerButton b) const {
            return !(m_CurrentButtons & static_cast<unsigned int>(b)) && (m_PreviousButtons & static_cast<unsigned int>(b));
        }
        bool IsPressed(ControllerButton b) const {
            return m_CurrentButtons & static_cast<unsigned int>(b);
        }

    private:
        unsigned int m_ControllerIndex;
        SDL_Gamepad* m_pGamepad;
        unsigned int m_CurrentButtons;
        unsigned int m_PreviousButtons;
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