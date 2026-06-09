#include "BonusOverlayComponent.h"

#include "Renderer.h"
#include "GameObject.h"

#include <SDL3/SDL.h>

namespace dae
{
    BonusOverlayComponent::BonusOverlayComponent(GameObject* owner)
        : Component(owner)
    {
    }

    void BonusOverlayComponent::SetActive(bool active)
    {
        m_Active = active;
    }

    void BonusOverlayComponent::SetAlpha(uint8_t alpha)
    {
        m_Alpha = alpha;
    }

    void BonusOverlayComponent::Render() const
    {
        if (!m_Active) return;

        SDL_Renderer* renderer = Renderer::GetInstance().GetSDLRenderer();

        SDL_FRect rect{ 0.f, 52.f, 1040.f, 560.f };

        uint8_t r{}, g{}, b{}, a{};
        SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, m_Alpha); // white at chosen alpha
        SDL_RenderFillRect(renderer, &rect);

        SDL_SetRenderDrawColor(renderer, r, g, b, a);
    }
}
