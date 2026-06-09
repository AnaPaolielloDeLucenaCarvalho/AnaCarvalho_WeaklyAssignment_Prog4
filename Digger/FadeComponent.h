#ifndef FADECOMPONENT_H
#define FADECOMPONENT_H

#include "Component.h"

#include <cstdint>

namespace dae
{
    // Fades the screen to black over the specified duration, then destroys itself.
    class FadeComponent final : public Component
    {
    public:
        explicit FadeComponent(GameObject* owner, float fadeDuration);

        void Update(float deltaTime) override;
        void Render() const override;

    private:
        float m_FadeDuration;
        float m_Timer{ 0.0f };
        uint8_t m_Alpha{ 0 };
    };
}

#endif // FADECOMPONENT_H