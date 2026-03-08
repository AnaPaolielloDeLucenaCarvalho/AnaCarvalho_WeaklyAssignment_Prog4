#ifndef FPSCOMPONENT_H // No #pragma once, use include guards (for tom's sake)
#define FPSCOMPONENT_H

#include "Component.h"

namespace dae
{
    class FPSComponent final : public Component
    {
    public:
        FPSComponent(GameObject* pOwner);
        ~FPSComponent() = default;

        void Update(float deltaTime) override;

    private:
        float m_timePassed{ 0.0f };
        int m_frameCount{ 0 };
    };
}

#endif // FPSCOMPONENT_H