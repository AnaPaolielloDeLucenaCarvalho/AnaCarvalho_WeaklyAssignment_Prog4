#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include "Singleton.h"
#include "Command.h"
#include "Gamepad.h"
#include <map>
#include <vector>
#include <memory>
#include <SDL3/SDL.h>


// DESIGN PATTERN - Command Pattern (Invoker)
// The InputManager polls SDL events and gamepads, but it doesn't know what the buttons actually do. It just looks up the bound Command object and tells it to Execute(), keeping hardware completely separate from gameplay logic.

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
        InputManager();

        bool ProcessInput(float deltaTime);

        void BindCommand(SDL_Scancode key, KeyState state, std::unique_ptr<Command> command);
        void BindCommand(unsigned int controllerIndex, Gamepad::ControllerButton button, KeyState state, std::unique_ptr<Command> command);

		// teacher feedback - unbind commands
        void UnbindAll();

    private:
        std::multimap<std::pair<SDL_Scancode, KeyState>, std::unique_ptr<Command>> m_keyboardCommands;
		std::vector<std::unique_ptr<Gamepad>> m_pGamepads; // so it supports multiple gamepads

        struct GamepadBinding
        {
            unsigned int controllerIndex;
            Gamepad::ControllerButton button;
            KeyState state;

            bool operator<(const GamepadBinding& other) const
            {
                return std::tie(controllerIndex, button, state) < std::tie(other.controllerIndex, other.button, other.state);
            }
        };
        std::multimap<GamepadBinding, std::unique_ptr<Command>> m_gamepadCommands;
    };
}

#endif