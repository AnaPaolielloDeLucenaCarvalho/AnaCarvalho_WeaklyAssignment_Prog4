#ifndef BONUS_UI_COMPONENT_H
#define BONUS_UI_COMPONENT_H

#include "Component.h"
#include <cstdint>

// DESIGN PATTERN - Component Pattern
// I made this a separate component instead of hardcoding the screen flash into the Map Manager. By isolating it, we can just attach this UI overlay to any empty GameObject and toggle its alpha transparency independently, keeping the rendering logic completely modular.

namespace dae
{
    class BonusUIComponent final : public Component
    {
    public:
        explicit BonusUIComponent(GameObject* owner);

        void Update(float deltaTime) override;
        void Render() const override;

        void SetActive(bool active);
        void SetAlpha(uint8_t alpha);

    private:
        bool m_Active{ false };
        uint8_t m_Alpha{ 90 }; // default - moderately visible white overlay
    };
}

#endif // BONUS_UI_COMPONENT_H
