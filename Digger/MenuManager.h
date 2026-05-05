#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include "Component.h"
#include "SceneManager.h"
#include <SDL3/SDL.h>

namespace dae
{
    class MenuManager : public Component
    {
    public:
        MenuManager(GameObject* owner, Scene* targetGameScene)
            : Component(owner), m_pTargetScene(targetGameScene) {
        }

        void Update(float /*deltaTime*/) override
        {
            // if any key is pressed we start the game
            auto pStates = SDL_GetKeyboardState(nullptr);

            for (int i = 0; i < SDL_SCANCODE_COUNT; ++i)
            {
                if (pStates[i])
                {
                    SceneManager::GetInstance().SetActiveScene(m_pTargetScene);
                    return;
                }
            }
        }

    private:
        Scene* m_pTargetScene;
    };
}
#endif