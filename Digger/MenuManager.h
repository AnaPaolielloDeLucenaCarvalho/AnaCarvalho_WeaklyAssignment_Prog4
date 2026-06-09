#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include "Component.h"

namespace dae
{
    class Scene; // forward declaration — full type only needed in .cpp

    class MenuManager final : public Component
    {
    public:
        MenuManager(GameObject* owner, Scene* targetGameScene);

        void Update(float deltaTime) override;

    private:
        Scene* m_pTargetScene;
    };
}

#endif // MENUMANAGER_H