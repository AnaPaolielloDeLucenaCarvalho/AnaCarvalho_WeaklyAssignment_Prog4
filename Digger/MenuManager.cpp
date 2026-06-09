#include "MenuManager.h"

#include "SceneManager.h"
#include "Scene.h"

#include <SDL3/SDL.h>

namespace dae
{
    MenuManager::MenuManager(GameObject* owner, Scene* targetGameScene)
        : Component(owner)
        , m_pTargetScene(targetGameScene)
    {
    }

    void MenuManager::Update(float /*deltaTime*/)
    {
        // Transition to the game scene on any keyboard input
        const bool* pStates = SDL_GetKeyboardState(nullptr);

        for (int i = 0; i < SDL_SCANCODE_COUNT; ++i)
        {
            if (pStates[i])
            {
                SceneManager::GetInstance().SetActiveScene(m_pTargetScene);
                return;
            }
        }
    }
}
