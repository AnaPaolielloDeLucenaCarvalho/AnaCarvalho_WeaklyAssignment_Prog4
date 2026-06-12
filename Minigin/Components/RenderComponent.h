#ifndef RENDERCOMPONENT_H // No #pragma once, use include guards (for tom's sake)
#define RENDERCOMPONENT_H

#include <string>
#include <memory>
#include <cstdint>
#include "Component.h"


// DESIGN PATTERN - Component Pattern
// Handles drawing 2D textures to the screen. Keeps the OpenGL/SDL rendering code out of the core GameObject class.

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
        void SetScale(float scale) { m_scale = scale; }
        void SetRotation(double angle) { m_angle = angle; }
        void SetColorMod(uint8_t r, uint8_t g, uint8_t b);

        void SetAdditiveBoost(bool enabled, uint8_t boostAlpha = 120);

    private:
        std::shared_ptr<Texture2D> m_texture;
        bool m_isFlipped{ false };
        float m_scale{ 1.0f };
        double m_angle{ 0.0 };
        bool m_AdditiveBoost{ false };
        uint8_t m_BoostAlpha{ 120 };
    };
}
#endif // RENDERCOMPONENT_H