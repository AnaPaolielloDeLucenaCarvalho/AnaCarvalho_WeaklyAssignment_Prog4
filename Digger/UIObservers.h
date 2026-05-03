#ifndef UIOBSERVERS_H
#define UIOBSERVERS_H

#include "Observer.h"
#include "Component.h"
#include "TextComponent.h"
#include "GameObject.h"
#include <string>
#include "Subject.h"

#include "Texture2D.h"
#include "ResourceManager.h"
#include "Renderer.h"

#include <vector>
#include <iomanip>
#include <sstream>

namespace dae
{
    class LivesDisplayComponent : public Component, public Observer
    {
    public:

		// feedback - add prefix
        LivesDisplayComponent(GameObject* pOwner, const std::string& prefix, int startLives) 
            : Component(pOwner)
            , m_Prefix(prefix)
            , m_lives(startLives) 
        {
        }

        void Update(float /*deltaTime*/) override {}

        void OnNotify(EventId eventId, int value) override
        {
            if (eventId == make_sdbm_hash("PlayerDied"))
            {
                m_lives = value;

                if (auto text = GetOwner()->GetComponent<TextComponent>())
                {
                    text->SetText(m_Prefix + "Lives: " + std::to_string(m_lives));
                }
            }
        }

    private:
        std::string m_Prefix;
        int m_lives;
    };

	// Sprite lives display instead of text!! draw a texture for each life
    class LivesSpriteDisplayComponent : public Component, public Observer
    {
    public:
        LivesSpriteDisplayComponent(GameObject* pOwner, const std::string& texturePath, int startLives, float spacing = 35.f)
            : Component(pOwner)
            , m_lives(startLives)
            , m_spacing(spacing)
        {
            m_texture = ResourceManager::GetInstance().LoadTexture(texturePath);
        }

        void Update(float /*deltaTime*/) override {}

        // draw the texture multiple times!
        void Render() const override
        {
            if (!m_texture) return;

            auto pos = GetOwner()->GetTransform().GetPosition();
            auto size = m_texture->GetSize();

			// -1 because we don't want to draw a life for the current life only the remaining ones
            for (int i = 0; i < m_lives - 1; ++i)
            {
                Renderer::GetInstance().RenderTexture(
                    *m_texture,
                    pos.x + (i * m_spacing),
                    pos.y,
                    size.x,
                    size.y
                );
            }
        }

        void OnNotify(EventId eventId, int value) override
        {
            if (eventId == make_sdbm_hash("PlayerDied"))
            {
                m_lives = value; // Update the when a life is lost
            }
        }

    private:
        std::shared_ptr<Texture2D> m_texture;
        int m_lives;
        float m_spacing;
    };

    class ScoreDisplayComponent : public Component, public Observer
    {
    public:
        ScoreDisplayComponent(GameObject* pOwner, const std::string& prefix)
            : Component(pOwner), m_Prefix(prefix), m_score(0) {
        }

        void AddObserver(Observer* obs) { m_Subject.AddObserver(obs); }

        void Update(float /*deltaTime*/) override {}

        void OnNotify(EventId eventId, int value) override
        {
            if (eventId == make_sdbm_hash("DiamondPickedUp"))
            {
                m_score += value;

                if (auto text = GetOwner()->GetComponent<TextComponent>())
                {
                    //text->SetText(m_Prefix + "Score: " + std::to_string(m_score));

					// instead of score: 0, we want score: 00000 - setw and setfill
                    std::ostringstream ss;
                    ss << m_Prefix << std::setw(5) << std::setfill('0') << m_score;
                    text->SetText(ss.str());
                }
                m_Subject.Notify(make_sdbm_hash("ScoreChanged"), m_score);
            }
        }

    private:
        std::string m_Prefix;
        int m_score;
        Subject m_Subject;
    };

	// Sprite score display instead of text! draw a texture for each digit
    class SpriteScoreDisplayComponent : public Component, public Observer
    {
    public:
        // adjust digitPrefix to match - PNG/UI/VNUM
        SpriteScoreDisplayComponent(GameObject* pOwner, const std::string& digitPrefix = "VNUM", const std::string& extension = ".png", float spacing = 24.f)
            : Component(pOwner)
            , m_score(0)
            , m_spacing(spacing)
        {
            for (int i = 0; i < 10; ++i)
            {
                std::string path = digitPrefix + std::to_string(i) + extension;
                m_digitTextures.push_back(ResourceManager::GetInstance().LoadTexture(path));
            }
        }

        void AddObserver(Observer* obs) { m_Subject.AddObserver(obs); }

        void Update(float /*deltaTime*/) override {}

        void Render() const override
        {
			// the score needs to be 5 digits always
            std::ostringstream ss;
            ss << std::setw(5) << std::setfill('0') << m_score;
            std::string scoreStr = ss.str();

            auto pos = GetOwner()->GetTransform().GetPosition();

            // draw the matching texture
            for (size_t i = 0; i < scoreStr.length(); ++i)
            {
                // convert char to int
                int digit = scoreStr[i] - '0';

                if (m_digitTextures[digit])
                {
                    auto size = m_digitTextures[digit]->GetSize();
                    Renderer::GetInstance().RenderTexture(
                        *m_digitTextures[digit],
                        pos.x + (i * m_spacing),
                        pos.y,
                        size.x,
                        size.y
                    );
                }
            }
        }

        void OnNotify(EventId eventId, int value) override
        {
            if (eventId == make_sdbm_hash("DiamondPickedUp"))
            {
                m_score += value;
                m_Subject.Notify(make_sdbm_hash("ScoreChanged"), m_score);
            }
        }

    private:
        std::vector<std::shared_ptr<Texture2D>> m_digitTextures;
        int m_score;
        float m_spacing;
        Subject m_Subject;
    };

	// A UI panel that draws a rectangle - in this case for the black border behind the score and lives, but can be used for anything
    class UIPanelComponent : public Component
    {
    public:
        UIPanelComponent(GameObject* pOwner, float width, float height, SDL_Color color = { 0, 0, 0, 255 })
            : Component(pOwner), m_width(width), m_height(height), m_color(color) {
        }

        void Update(float /*deltaTime*/) override {}

        void Render() const override
        {
            auto pos = GetOwner()->GetTransform().GetPosition();
            SDL_FRect rect{ pos.x, pos.y, m_width, m_height };

            auto renderer = Renderer::GetInstance().GetSDLRenderer();

            // Draw the colored rectangle
            SDL_SetRenderDrawColor(renderer, m_color.r, m_color.g, m_color.b, m_color.a);
            SDL_RenderFillRect(renderer, &rect);
        }

    private:
        float m_width;
        float m_height;
        SDL_Color m_color;
    };
}
#endif