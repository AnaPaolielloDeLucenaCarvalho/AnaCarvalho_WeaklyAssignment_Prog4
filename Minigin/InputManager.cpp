#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#include <algorithm>
#include "InputManager.h"

namespace dae
{
    void InputManager::BindCommand(SDL_Scancode key, KeyState state, std::unique_ptr<Command> command)
    {
        m_KeyboardCommands[std::make_pair(key, state)] = std::move(command);
    }

    void InputManager::BindCommand(Gamepad::ControllerButton button, KeyState state, std::unique_ptr<Command> command)
    {
        m_GamepadCommands[std::make_pair(button, state)] = std::move(command);
    }

    bool InputManager::ProcessInput()
    {
        m_pGamepad->Update();

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
                command->Execute();
            }
        }

        // Gamepad Commands
        std::for_each(m_GamepadCommands.begin(), m_GamepadCommands.end(), [this](auto& pair)
        {
            const auto& [button, state] = pair.first;
            bool shouldExecute = false;

            switch (state)
            {
            case KeyState::Down:    
                shouldExecute = m_pGamepad->IsDown(button);
                break;
            case KeyState::Up:      
                shouldExecute = m_pGamepad->IsUp(button); 
                break;
            case KeyState::Pressed: 
                shouldExecute = m_pGamepad->IsPressed(button); 
                break;
            }

            if (shouldExecute)
            {
                pair.second->Execute();
            }
        });

        return true;
    }
}