#ifndef FADECOMPONENT_H
#define FADECOMPONENT_H

#include "Component.h"
#include "GameObject.h"
#include "Renderer.h"
#include <SDL3/SDL.h>
#include <algorithm>

namespace dae
{
    class FadeComponent : public Component
    {
    public:
        FadeComponent(GameObject* owner, float fadeDuration)
            : Component(owner), m_FadeDuration(fadeDuration)
        {
        }

        void Update(float deltaTime) override
        {
            m_Timer += deltaTime;
            m_Alpha = static_cast<Uint8>(std::min(255.0f, (m_Timer / m_FadeDuration) * 255.0f));

            if (m_Timer >= m_FadeDuration + 0.1f)
            {
                GetOwner()->MarkForDestroy();
            }
        }

        void Render() const override
        {
            auto renderer = Renderer::GetInstance().GetSDLRenderer();

            // blend mode allow transparency
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

            // black rectangle with alpha = fade effect
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, m_Alpha);
            SDL_FRect rect{ 0, 0, 1040.f, 612.f };
            SDL_RenderFillRect(renderer, &rect);

            // blend mode to default
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }

    private:
        float m_FadeDuration;
        float m_Timer{ 0.0f };
        Uint8 m_Alpha{ 0 };
    };
}
#endif