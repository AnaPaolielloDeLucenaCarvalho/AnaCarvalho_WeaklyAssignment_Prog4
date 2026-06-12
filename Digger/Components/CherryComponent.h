#ifndef CHERRYCOMPONENT_H
#define CHERRYCOMPONENT_H

#include "Component.h"
#include "GameObject.h"

// DESIGN PATTERN - Component Pattern
// By keeping the Cherry logic encapsulated in its own component, it doesn't clutter the LevelManager. It independently handles its own lifespan timer and collision checks, making it a true plug-and-play entity.

namespace dae
{
    class DiggerComponent;

    // Cherry pickup — spawned by LevelTransitionManager after delay at 'B' (bonus) position. When Digger walks over it, Bonus Mode is activated (15s) - cherry auto-destroys itself after m_Lifetime seconds if not collected.
    class CherryComponent final : public Component
    {
    public:
        CherryComponent(GameObject* owner, DiggerComponent* p1, DiggerComponent* p2 = nullptr, float lifetime = 10.0f);

        void Update(float deltaTime) override;

    private:
        DiggerComponent* m_p1;
        DiggerComponent* m_p2;
        float m_Lifetime;

        // checks whether pDigger has collected this cherry
        bool CheckPickup(DiggerComponent* pDigger, float myX, float myY) const;
    };
}

#endif // CHERRYCOMPONENT_H
