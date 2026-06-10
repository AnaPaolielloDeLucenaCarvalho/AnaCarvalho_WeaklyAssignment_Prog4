#ifndef FIREBALLCOMPONENT_H
#define FIREBALLCOMPONENT_H

#include "Component.h"

#include <glm/vec2.hpp>

namespace dae
{
    class DiggerComponent; // forward declaration — full type only needed in .cpp

    class FireballComponent final : public Component
    {
    public:
        FireballComponent(GameObject* owner, const glm::vec2& dir, DiggerComponent* pDigger);

        void Update(float deltaTime) override;

    private:
        glm::vec2 m_Direction;
        float m_AnimTimer{ 0.0f };
        int m_Frame{ 1 };

        bool m_IsExploding{ false };
        float m_ExplodeTimer{ 0.0f };
        int m_ExplodeFrame{ 1 };

        DiggerComponent* m_pDigger;
    };
}

#endif // FIREBALLCOMPONENT_H