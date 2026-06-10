#ifndef BONUS_OVERLAY_COMPONENT_H
#define BONUS_OVERLAY_COMPONENT_H

#include "Component.h"
#include <cstdint>

namespace dae
{
    class BonusOverlayComponent final : public Component
    {
    public:
        explicit BonusOverlayComponent(GameObject* owner);

        void Render() const override;

        void SetActive(bool active);
        void SetAlpha(uint8_t alpha);

    private:
        bool m_Active{ false };
        uint8_t m_Alpha{ 90 }; // default - moderately visible white overlay
    };
}

#endif // BONUS_OVERLAY_COMPONENT_H
