#include "FPSComponent.h"
#include "GameObject.h"
#include "TextComponent.h"
#include <string>

namespace dae
{
    FPSComponent::FPSComponent(GameObject* pOwner)
        : Component(pOwner)
    {
    }

    void FPSComponent::Update(float deltaTime)
    {
        m_timePassed += deltaTime;
        m_frameCount++;

        // Update text every second
        if (m_timePassed >= 1.0f)
        {
            int fps = m_frameCount;

            auto pTextComponent = GetOwner()->GetComponent<TextComponent>();
            if (pTextComponent != nullptr)
            {
                pTextComponent->SetText(std::to_string(fps) + " FPS");
            }

            // Reset counters
            m_timePassed -= 1.0f;
            m_frameCount = 0;
        }
    }
}