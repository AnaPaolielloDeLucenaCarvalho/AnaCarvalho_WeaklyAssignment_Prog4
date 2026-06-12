#ifndef TEXTCOMPONENT_H // No #pragma once, use include guards (for tom's sake)
#define TEXTCOMPONENT_H

#include <string>
#include <memory>
#include "RenderComponent.h"
#include <SDL3/SDL.h>

namespace dae
{
    class Font;

    class TextComponent final : public RenderComponent
    {
    public:
        TextComponent(GameObject* pOwner, const std::string& text, std::shared_ptr<Font> font, const SDL_Color& color = { 255, 255, 255, 255 });
        virtual ~TextComponent() = default;

        void Update(float deltaTime) override;

        void SetText(const std::string& text);
        void SetColor(const SDL_Color& color);

    private:
        bool m_needsUpdate;
        std::string m_text;
        std::shared_ptr<Font> m_font;
        SDL_Color m_color;
    };
}

#endif // TEXTCOMPONENT_H