#ifndef RENDERCOMPONENT_H // No #pragma once, use include guards (for tom's sake)
#define RENDERCOMPONENT_H

#include <string>
#include <memory>
#include "Component.h"

namespace dae
{
    class Texture2D;

    class RenderComponent : public Component
    {
    public:
        RenderComponent(GameObject* pOwner);
        RenderComponent(GameObject* pOwner, const std::string& filename);
        ~RenderComponent() = default;

        void Update(float deltaTime) override;
        void Render() const override;

        void SetTexture(const std::string& filename);
        void SetTexture(std::shared_ptr<Texture2D> texture) { m_texture = std::move(texture); };

        void SetFlip(bool flip) { m_isFlipped = flip; }
        void SetScale(float scale) { m_Scale = scale; }

    private:
        std::shared_ptr<Texture2D> m_texture;
        bool m_isFlipped{ false };
        float m_Scale{ 1.0f };
    };
}
#endif // RENDERCOMPONENT_H