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
        float scaledW = size.x * m_Scale;
        float scaledH = size.y * m_Scale;

        const auto flip = m_isFlipped ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        dae::Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y, scaledW, scaledH, flip);
    }

    void RenderComponent::SetTexture(const std::string& filename)
    {
        m_texture = ResourceManager::GetInstance().LoadTexture(filename);
    }
}