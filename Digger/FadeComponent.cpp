#include "FadeComponent.h"

#include "GameObject.h"
#include "Renderer.h"

#include <SDL3/SDL.h>
#include <algorithm>

namespace dae
{
    FadeComponent::FadeComponent(GameObject* owner, float fadeDuration)
        : Component(owner)
        , m_FadeDuration(fadeDuration)
    {
    }

    void FadeComponent::Update(float deltaTime)
    {
        m_Timer += deltaTime;
        m_Alpha = static_cast<uint8_t>(
            std::min(255.0f, (m_Timer / m_FadeDuration) * 255.0f)
        );

        if (m_Timer >= m_FadeDuration + 0.1f)
        {
            GetOwner()->MarkForDestroy();
        }
    }

    void FadeComponent::Render() const
    {
        auto* renderer = Renderer::GetInstance().GetSDLRenderer();

        // Enable alpha blending so the black overlay is semi-transparent during the fade
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, m_Alpha);

        SDL_FRect rect{ 0.f, 0.f, 1040.f, 612.f };
        SDL_RenderFillRect(renderer, &rect);

        // Restore the default blend mode
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }
}
