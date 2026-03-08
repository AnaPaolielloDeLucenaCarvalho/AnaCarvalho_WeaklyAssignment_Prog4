#include "RenderComponent.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "GameObject.h"

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

    void RenderComponent::Render() const
    {
        const auto& pos = GetOwner()->GetTransform().GetPosition();
        Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y);
    }

    void RenderComponent::SetTexture(const std::string& filename)
    {
        m_texture = ResourceManager::GetInstance().LoadTexture(filename);
    }
}