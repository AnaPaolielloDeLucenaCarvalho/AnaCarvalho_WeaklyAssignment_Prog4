#include "UIObservers.h"

#include "TextComponent.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "LevelManager.h"

#include <iomanip>
#include <sstream>
#include <string>

namespace dae
{
// LivesDisplayComponent
    LivesDisplayComponent::LivesDisplayComponent(GameObject* pOwner, const std::string& prefix, int startLives)
        : Component(pOwner)
        , m_Prefix(prefix)
        , m_lives(startLives)
    {
    }

    void LivesDisplayComponent::Update(float /*deltaTime*/) {}

    void LivesDisplayComponent::OnNotify(EventId eventId, int value)
    {
        // Only react if the specific event we care about is broadcasted
        if (eventId == make_sdbm_hash("PlayerDied"))
        {
            m_lives = value;

            // Grab the sibling TextComponent and directly update its string cache
            if (auto* text = GetOwner()->GetComponent<TextComponent>())
            {
                text->SetText(m_Prefix + "Lives: " + std::to_string(m_lives));
            }
        }
    }

// LivesSpriteDisplayComponent
    LivesSpriteDisplayComponent::LivesSpriteDisplayComponent(GameObject* pOwner, const std::string& texturePath, int startLives, float spacing, bool drawLeft)
        : Component(pOwner)
        , m_lives(startLives)
        , m_spacing(spacing)
        , m_DrawLeft(drawLeft)
    {
        m_texture = ResourceManager::GetInstance().LoadTexture(texturePath);
    }

    void LivesSpriteDisplayComponent::SetTexture(const std::string& texturePath)
    {
        m_texture = ResourceManager::GetInstance().LoadTexture(texturePath);
    }

    void LivesSpriteDisplayComponent::Update(float /*deltaTime*/) {}

    void LivesSpriteDisplayComponent::Render() const
    {
        if (!m_texture) return;

        const auto pos = GetOwner()->GetTransform().GetPosition();
        const auto size = m_texture->GetSize();

        // Draw reserve lives dynamically based on the current lives count.
        // Subtract 1 because the current active life doesn't need a UI sprite.
        for (int i = 0; i < m_lives - 1; ++i)
        {
            float xPos = pos.x + (static_cast<float>(i) * m_spacing);

            // Adjust the drawing direction based on UI placement (Ex - Player 2 draws right-to-left)
            if (m_DrawLeft)
            {
                xPos = pos.x - (static_cast<float>(i) * m_spacing);
            }

            Renderer::GetInstance().RenderTexture(
                *m_texture,
                xPos,
                pos.y,
                size.x,
                size.y
            );
        }
    }

    void LivesSpriteDisplayComponent::OnNotify(EventId eventId, int value)
    {
        if (eventId == make_sdbm_hash("PlayerDied"))
        {
            m_lives = value;
        }
    }

// ScoreDisplayComponent
    ScoreDisplayComponent::ScoreDisplayComponent(GameObject* pOwner, const std::string& prefix)
        : Component(pOwner)
        , m_Prefix(prefix)
        , m_score(0)
    {
    }

    void ScoreDisplayComponent::AddObserver(Observer* obs)
    {
        m_Subject.AddObserver(obs);
    }

    void ScoreDisplayComponent::Update(float /*deltaTime*/) {}

    void ScoreDisplayComponent::OnNotify(EventId eventId, int value)
    {
        if (eventId == make_sdbm_hash("DiamondPickedUp"))
        {
            m_score += value;

            if (auto* text = GetOwner()->GetComponent<TextComponent>())
            {
                // Format the score with leading zeros (Ex - 00025) to match the arcade style
                std::ostringstream ss;
                ss << m_Prefix << std::setw(5) << std::setfill('0') << m_score;
                text->SetText(ss.str());
            }

            // Re-broadcast the updated score to any systems that care (like the HighScoreManager)
            m_Subject.Notify(make_sdbm_hash("ScoreChanged"), m_score);
        }
    }

// SpriteScoreDisplayComponent
    SpriteScoreDisplayComponent::SpriteScoreDisplayComponent(GameObject* pOwner, const std::string& digitPrefix, const std::string& extension, float spacing)
        : Component(pOwner)
        , m_score(0)
        , m_spacing(spacing)
    {
        // Pre-load all 10 digit textures (0-9) into memory once during initialization so we don't have to load files from the hard drive every single frame.
        for (int i = 0; i < 10; ++i)
        {
            const std::string path = digitPrefix + std::to_string(i) + extension;
            m_digitTextures.push_back(ResourceManager::GetInstance().LoadTexture(path));
        }
    }

    void SpriteScoreDisplayComponent::AddObserver(Observer* obs)
    {
        m_Subject.AddObserver(obs);
    }

    void SpriteScoreDisplayComponent::Update(float /*deltaTime*/) {}

    void SpriteScoreDisplayComponent::Render() const
    {
        // Hide scores entirely in Versus mode
        if (LevelManager::GetInstance().GetGameMode() == GameMode::Versus) return;

        // Convert the integer score into a 5-digit zero-padded string for easy character iteration
        std::ostringstream ss;
        ss << std::setw(5) << std::setfill('0') << m_score;
        const std::string scoreStr = ss.str();

        const auto pos = GetOwner()->GetTransform().GetPosition();

        // Loop through each character in the formatted string, map it to our pre-loaded texture array, and draw it
        for (size_t i = 0; i < scoreStr.size(); ++i)
        {
            const int digit = scoreStr[i] - '0';
            if (m_digitTextures[digit])
            {
                const auto size = m_digitTextures[digit]->GetSize();
                Renderer::GetInstance().RenderTexture(
                    *m_digitTextures[digit],
                    pos.x + (static_cast<float>(i) * m_spacing),
                    pos.y,
                    size.x,
                    size.y
                );
            }
        }
    }

    void SpriteScoreDisplayComponent::OnNotify(EventId eventId, int value)
    {
        if (eventId == make_sdbm_hash("DiamondPickedUp"))
        {
            m_score += value;
            m_Subject.Notify(make_sdbm_hash("ScoreChanged"), m_score);
        }
        else if (eventId == make_sdbm_hash("ScoreReset"))
        {
            m_score = value;
            m_Subject.Notify(make_sdbm_hash("ScoreChanged"), m_score);
        }
    }

    // UIPanelComponent
    UIPanelComponent::UIPanelComponent(GameObject* pOwner, float width, float height, SDL_Color color)
        : Component(pOwner)
        , m_width(width)
        , m_height(height)
        , m_color(color)
    {
    }

    void UIPanelComponent::Update(float /*deltaTime*/) {}

    void UIPanelComponent::Render() const
    {
        // Draw a solid background panel behind UI text so it doesn't get lost in the game world
        const auto pos = GetOwner()->GetTransform().GetPosition();
        SDL_FRect  rect{ pos.x, pos.y, m_width, m_height };

        auto* renderer = Renderer::GetInstance().GetSDLRenderer();
        SDL_SetRenderDrawColor(renderer, m_color.r, m_color.g, m_color.b, m_color.a);
        SDL_RenderFillRect(renderer, &rect);
    }
}
