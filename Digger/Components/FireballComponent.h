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

        void Render() const override {}
        void Update(float deltaTime) override;

    private:
        glm::vec2 m_direction;
        float m_animTimer{ 0.0f };
        int m_frame{ 1 };

        bool m_isExploding{ false };
        float m_explodeTimer{ 0.0f };
        int m_explodeFrame{ 1 };

        // DESIGN DECISION - Pointer referencing
        // The fireball stores a pointer to the specific player who shot it so it knows who to award points to!
        DiggerComponent* m_pDigger;
    };
}

#endif // FIREBALLCOMPONENT_H