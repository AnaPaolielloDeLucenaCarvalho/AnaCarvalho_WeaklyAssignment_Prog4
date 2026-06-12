#ifndef BONUS_UI_COMPONENT_H
#define BONUS_UI_COMPONENT_H

#include "Component.h"
#include <cstdint>

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
