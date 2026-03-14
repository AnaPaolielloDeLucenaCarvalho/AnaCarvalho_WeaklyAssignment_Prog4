#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#include <algorithm>
#include "InputManager.h"

namespace dae
{
    InputManager::InputManager()
    {
        // Player 1 and Player 2
        m_pGamepads.push_back(std::make_unique<Gamepad>(0));
        m_pGamepads.push_back(std::make_unique<Gamepad>(1));
    }

    void InputManager::BindCommand(SDL_Scancode key, KeyState state, std::unique_ptr<Command> command)
    {
        m_KeyboardCommands[std::make_pair(key, state)] = std::move(command);
    }

    void InputManager::BindCommand(unsigned int controllerIndex, Gamepad::ControllerButton button, KeyState state, std::unique_ptr<Command> command)
    {
        m_GamepadCommands[GamepadBinding{ controllerIndex, button, state }] = std::move(command);
    }

    void InputManager::UnbindAll()
    {
        m_KeyboardCommands.clear();
        m_GamepadCommands.clear();
    }

    bool InputManager::ProcessInput(float deltaTime)
    {
		// now it supports multiple gamepads
        for (auto& gamepad : m_pGamepads)
        {
            gamepad->Update();
        }

        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_EVENT_QUIT)
            {
                return false;
            }
            // IMGUI
            ImGui_ImplSDL3_ProcessEvent(&e);
        }

        // Keyboard Commands
        const bool* state = SDL_GetKeyboardState(nullptr);
        for (auto& [keyBinding, command] : m_KeyboardCommands)
        {
            const auto& [key, keyState] = keyBinding;
            if (keyState == KeyState::Pressed && state[key])
            {
                command->Execute(deltaTime);
            }
        }

        // Gamepad Commands
        for (auto& [binding, command] : m_GamepadCommands)
        {
            if (binding.controllerIndex < m_pGamepads.size())
            {
                auto& gamepad = m_pGamepads[binding.controllerIndex];
                bool shouldExecute = false;
                switch (binding.state)
                {
                case KeyState::Down:    
                    shouldExecute = gamepad->IsDown(binding.button); 
                    break;
                case KeyState::Up:      
                    shouldExecute = gamepad->IsUp(binding.button); 
                    break;
                case KeyState::Pressed: 
                    shouldExecute = gamepad->IsPressed(binding.button); 
                    break;
                }

                if (shouldExecute)
                {
                    command->Execute(deltaTime);
                }
            }
        }

        return true;
    }
}