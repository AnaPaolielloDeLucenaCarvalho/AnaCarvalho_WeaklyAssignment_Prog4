#include "RenderComponent.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "GameObject.h"
#include "Texture2D.h"

namespace dae
{
    RenderComponent::RenderComponent(GameObject* pOwner)
        : Component(pOwner), m_texture(nullptr)
    {
    }

    RenderComponent::RenderComponent(GameObject* pOwner, const std::string& filename)
        : Component(pOwner)
    {
        SetTexture(filename);
    }

    void RenderComponent::Update(float /*deltaTime*/) {}

    void dae::RenderComponent::Render() const
    {
        if (m_texture == nullptr) return;

        const auto& pos = GetOwner()->GetTransform().GetPosition();
        auto size = m_texture->GetSize();

        // Use the member variable scale
        float scaledW = size.x * m_scale;
        float scaledH = size.y * m_scale;

        const auto flip = m_isFlipped ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

        SDL_Texture* sdlTex = m_texture->GetSDLTexture();
        SDL_Renderer* sdlRen = dae::Renderer::GetInstance().GetSDLRenderer();
        SDL_FRect dst{ pos.x, pos.y, scaledW, scaledH };

        SDL_RenderTextureRotated(sdlRen, sdlTex, nullptr, &dst, m_angle, nullptr, flip);

        if (m_additiveBoost)
        {
            SDL_SetTextureBlendMode(sdlTex, SDL_BLENDMODE_ADD);
            SDL_SetTextureAlphaMod(sdlTex,  m_boostAlpha);
            SDL_SetTextureColorMod(sdlTex,  255, 255, 255);

            SDL_RenderTextureRotated(sdlRen, sdlTex, nullptr, &dst, m_angle, nullptr, flip);

            SDL_SetTextureBlendMode(sdlTex, SDL_BLENDMODE_BLEND);
            SDL_SetTextureAlphaMod(sdlTex,  255);
            SDL_SetTextureColorMod(sdlTex,  255, 255, 255);
        }
    }

    void RenderComponent::SetTexture(const std::string& filename)
    {
        m_texture = ResourceManager::GetInstance().LoadTexture(filename);
    }

    void RenderComponent::SetTexture(std::shared_ptr<Texture2D> texture)
    {
        m_texture = std::move(texture);
    }

    void RenderComponent::SetFlip(bool flip)
    {
        m_isFlipped = flip;
    }

    void RenderComponent::SetScale(float scale)
    {
        m_scale = scale;
    }

    void RenderComponent::SetRotation(double angle)
    {
        m_angle = angle;
    }

    void RenderComponent::SetColorMod(uint8_t r, uint8_t g, uint8_t b)
    {
        if (m_texture)
            SDL_SetTextureColorMod(m_texture->GetSDLTexture(), r, g, b);
    }

    void RenderComponent::SetAdditiveBoost(bool enabled, uint8_t boostAlpha)
    {
        m_additiveBoost = enabled;
        m_boostAlpha    = boostAlpha;
    }
}