#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include "Singleton.h"
#include "Command.h"
#include "Gamepad.h"
#include <map>
#include <vector>
#include <memory>
#include <SDL3/SDL.h>

namespace dae
{
    enum class KeyState
    { 
		Down, // pressed
		Up, // released
		Pressed // held
    };

    class InputManager final : public Singleton<InputManager>
    {
    public:
        bool ProcessInput();

        void BindCommand(SDL_Scancode key, KeyState state, std::unique_ptr<Command> command);
        void BindCommand(Gamepad::ControllerButton button, KeyState state, std::unique_ptr<Command> command);

    private:
        std::map<std::pair<SDL_Scancode, KeyState>, std::unique_ptr<Command>> m_KeyboardCommands;
        std::unique_ptr<Gamepad> m_pGamepad = std::make_unique<Gamepad>(0);
        std::map<std::pair<Gamepad::ControllerButton, KeyState>, std::unique_ptr<Command>> m_GamepadCommands;
    };
}

#endif