#ifndef FIREBALLCOMPONENT_H
#define FIREBALLCOMPONENT_H

#include "Component.h"
#include <glm/vec2.hpp>

// DESIGN PATTERN - Component Pattern (Composition over Inheritance)
// Instead of creating a messy inheritance tree (e.g., class Fireball : public Projectile),  we snap this isolated behaviour onto a hollow GameObject case.

namespace dae
{
    class DiggerComponent; // Forward declaration prevents cascading compile times

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

        // DESIGN DECISION - Pointer referencing
        // The fireball stores a pointer to the specific player who shot it so it knows who to award points to!
        DiggerComponent* m_pDigger;
    };
}

#endif // FIREBALLCOMPONENT_H