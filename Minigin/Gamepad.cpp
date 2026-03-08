#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Xinput.h>

#pragma comment(lib, "xinput.lib")

#include "Gamepad.h"

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

    Gamepad::Gamepad(unsigned int index) : m_pImpl(std::make_unique<GamepadImpl>(index)) {}
    Gamepad::~Gamepad() = default;
    void Gamepad::Update() { m_pImpl->Update(); }
    bool Gamepad::IsDown(ControllerButton button) const { return m_pImpl->IsDown(button); }
    bool Gamepad::IsUp(ControllerButton button) const { return m_pImpl->IsUp(button); }
    bool Gamepad::IsPressed(ControllerButton button) const { return m_pImpl->IsPressed(button); }
}