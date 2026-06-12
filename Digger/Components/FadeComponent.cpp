#include "FadeComponent.h"

#include "GameObject.h"
#include "Renderer.h"

#include <SDL3/SDL.h>
#include <algorithm>

namespace dae
{
    FadeComponent::FadeComponent(GameObject* owner, float fadeDuration)
        : Component(owner)
        , m_fadeDuration(fadeDuration)
    {
    }

    void FadeComponent::Update(float deltaTime)
    {
        // Linearly interpolate the alpha value from 0 to 255 based on elapsed time
        m_timer += deltaTime;
        m_alpha = static_cast<uint8_t>
            (
                std::min(255.0f, (m_timer / m_fadeDuration) * 255.0f)
            );

        // Cleanup - Once the fade is completely finished (with a small 0.1s buffer), we mark this object for destruction so it doesn't linger in memory forever.
        if (m_timer >= m_fadeDuration + 0.1f)
        {
            GetOwner()->MarkForDestroy();
        }
    }

    void FadeComponent::Render() const
    {
        auto* renderer = Renderer::GetInstance().GetSDLRenderer();

        // Enable alpha blending so the black overlay is semi-transparent during the fade
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, m_alpha);

        // Draw a massive black rectangle covering the entire arcade screen resolution
        SDL_FRect rect{ 0.f, 0.f, 1040.f, 612.f };
        SDL_RenderFillRect(renderer, &rect);

        // Restore the default blend mode so we don't accidentally make the rest of the game transparent
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }
}
