#include "MenuManager.h"

#include "HighScoreManager.h"
#include "SceneManager.h"
#include "Scene.h"
#include "LevelManager.h"

#include <SDL3/SDL.h>
#include <sstream>
#include <iomanip>

namespace dae
{
    MenuManager::MenuManager(GameObject* owner, HighScoreManager* pMgr, Scene* pScoreScene, Scene* pGameScene, const std::vector<TextComponent*>& options, const std::vector<TextComponent*>& scoreTexts, const std::vector<TextComponent*>& nameTexts)
        : Component(owner)
        , m_pMgr(pMgr)
        , m_pScoreScene(pScoreScene)
        , m_pGameScene(pGameScene)
        , m_options(options)
        , m_scoreTexts(scoreTexts)
        , m_nameTexts(nameTexts)
    {
    }

    void MenuManager::Update(float deltaTime)
    {
        // ARCHITECTURAL DECISION - Polling Timer
        // Reading from a text file on the hard drive is incredibly slow. Instead of forcing the game  to read the HighScores file 60 times a second in the Update loop, we only read it once  every 1.0 second. This keeps the framerate perfectly smooth while still keeping the board live.
        m_refreshTimer -= deltaTime;
        if (m_refreshTimer <= 0.0f && m_pMgr)
        {
            m_refreshTimer = 1.0f; // Reset the timer
            auto topScores = m_pMgr->GetTopScores(5);
            for (size_t i = 0; i < topScores.size() && i < m_scoreTexts.size(); ++i)
            {
                // Format the scores with leading zeros to match the arcade aesthetic
                std::ostringstream scoreOss;
                scoreOss << std::setfill('0') << std::setw(5) << topScores[i].score;
                if (m_scoreTexts[i]) m_scoreTexts[i]->SetText(scoreOss.str());
                if (m_nameTexts[i]) m_nameTexts[i]->SetText(topScores[i].initials);
            }
        }

        // DESIGN DECISION - Data-Driven UI
        // By storing the menu options in a std::vector, we can loop through them dynamically, cleaner than writing a giant hardcoded switch statement to color each option individually
        for (int i = 0; i < static_cast<int>(m_options.size()); ++i)
        {
            if (i == m_selectedIndex)
            {
                m_options[i]->SetColor(SDL_Color{ 255, 255, 0, 255 }); // Highlight selected in Yellow
            }
            else
            {
                m_options[i]->SetColor(SDL_Color{ 255, 255, 255, 255 }); // Unselected remains White
            }
        }
    }

    void MenuManager::NavigateUp()
    {
        // Decrement index and wrap around to the bottom of the list if we go past the top
        m_selectedIndex--;
        if (m_selectedIndex < 0) m_selectedIndex = static_cast<int>(m_options.size()) - 1;
    }

    void MenuManager::NavigateDown()
    {
        // Increment index and wrap around to the top of the list if we go past the bottom
        m_selectedIndex++;
        if (m_selectedIndex >= static_cast<int>(m_options.size())) m_selectedIndex = 0;
    }

    void MenuManager::Select()
    {
        // DESIGN PATTERN - Singleton access
        // We inform the global LevelManager that a completely fresh game session is starting, so it knows to completely wipe out any lingering states from previous playthroughs.
        LevelManager::GetInstance().SetNeedsGameReset(true);

        if (m_selectedIndex == 2) // "VERSUS" Mode
        {
            LevelManager::GetInstance().SetGameMode(GameMode::Versus);
            SceneManager::GetInstance().SetActiveScene(m_pGameScene);
        }
        else // "SINGLE PLAYER" (0) or "CO-OP" (1)
        {
            LevelManager::GetInstance().SetGameMode(m_selectedIndex == 0 ? GameMode::SinglePlayer : GameMode::CoOp);

            // STATE ROUTING: If the player hasn't entered their initials yet this session, intercept them and route  them to the Name Entry scene. Otherwise, drop them directly into the Game scene.
            if (m_pMgr && !m_pMgr->HasSessionName())
            {
                SceneManager::GetInstance().SetActiveScene(m_pScoreScene);
            }
            else
            {
                SceneManager::GetInstance().SetActiveScene(m_pGameScene);
            }
        }
    }
}
