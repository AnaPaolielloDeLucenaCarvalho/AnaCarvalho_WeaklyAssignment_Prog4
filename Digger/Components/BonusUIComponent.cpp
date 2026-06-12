#include "BonusUIComponent.h"

#include "Renderer.h"
#include "GameObject.h"

#include <SDL3/SDL.h>

namespace dae
{
    BonusUIComponent::BonusUIComponent(GameObject* owner)
        : Component(owner)
    {
    }

    void BonusUIComponent::Update(float /*deltaTime*/)
    {
    }

    void BonusUIComponent::SetActive(bool active)
    {
        m_Active = active;
    }

    void BonusUIComponent::SetAlpha(uint8_t alpha)
    {
        m_Alpha = alpha;
    }

    void BonusUIComponent::Render() const
    {
        // Early out to save rendering performance if the bonus UI isn't currently triggered
        if (!m_Active) return;

        SDL_Renderer* renderer = Renderer::GetInstance().GetSDLRenderer();

        // Hardcoded dimensions to cover the play area grid perfectly (skipping the top UI bar)
        SDL_FRect rect{ 0.f, 52.f, 1040.f, 560.f };

        // Cache the original render color so we can safely restore it after drawing our semi-transparent overlay
        uint8_t r{}, g{}, b{}, a{};
        SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);

        // Enable alpha blending to allow the game world to show through the white flash
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, m_Alpha); // white at chosen alpha
        SDL_RenderFillRect(renderer, &rect);

        // Restore the engine's original drawing color so we don't break subsequent render calls
        SDL_SetRenderDrawColor(renderer, r, g, b, a);
    }
}
