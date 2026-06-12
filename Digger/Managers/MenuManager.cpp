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
        , m_Options(options)
        , m_ScoreTexts(scoreTexts)
        , m_NameTexts(nameTexts)
    {
    }

    void MenuManager::Update(float deltaTime)
    {
        // ARCHITECTURAL DECISION - Polling Timer
        // Reading from a text file on the hard drive is incredibly slow. Instead of forcing the game  to read the HighScores file 60 times a second in the Update loop, we only read it once  every 1.0 second. This keeps the framerate perfectly smooth while still keeping the board live.
        m_RefreshTimer -= deltaTime;
        if (m_RefreshTimer <= 0.0f && m_pMgr)
        {
            m_RefreshTimer = 1.0f; // Reset the timer
            auto topScores = m_pMgr->GetTopScores(5);
            for (size_t i = 0; i < topScores.size() && i < m_ScoreTexts.size(); ++i)
            {
                // Format the scores with leading zeros to match the arcade aesthetic
                std::ostringstream scoreOss;
                scoreOss << std::setfill('0') << std::setw(5) << topScores[i].score;
                if (m_ScoreTexts[i]) m_ScoreTexts[i]->SetText(scoreOss.str());
                if (m_NameTexts[i]) m_NameTexts[i]->SetText(topScores[i].initials);
            }
        }

        // DESIGN DECISION - Data-Driven UI
        // By storing the menu options in a std::vector, we can loop through them dynamically, cleaner than writing a giant hardcoded switch statement to color each option individually
        for (int i = 0; i < static_cast<int>(m_Options.size()); ++i)
        {
            if (i == m_SelectedIndex)
            {
                m_Options[i]->SetColor(SDL_Color{ 255, 255, 0, 255 }); // Highlight selected in Yellow
            }
            else
            {
                m_Options[i]->SetColor(SDL_Color{ 255, 255, 255, 255 }); // Unselected remains White
            }
        }
    }

    void MenuManager::NavigateUp()
    {
        // Decrement index and wrap around to the bottom of the list if we go past the top
        m_SelectedIndex--;
        if (m_SelectedIndex < 0) m_SelectedIndex = 2;
    }

    void MenuManager::NavigateDown()
    {
        // Increment index and wrap around to the top of the list if we go past the bottom
        m_SelectedIndex++;
        if (m_SelectedIndex > 2) m_SelectedIndex = 0;
    }

    void MenuManager::Select()
    {
        // DESIGN PATTERN - Singleton access
        // We inform the global LevelManager that a completely fresh game session is starting, so it knows to completely wipe out any lingering states from previous playthroughs.
        LevelManager::GetInstance().SetNeedsGameReset(true);

        if (m_SelectedIndex == 2) // "VERSUS" Mode
        {
            LevelManager::GetInstance().SetGameMode(GameMode::Versus);
            SceneManager::GetInstance().SetActiveScene(m_pGameScene);
        }
        else // "SINGLE PLAYER" (0) or "CO-OP" (1)
        {
            LevelManager::GetInstance().SetGameMode(m_SelectedIndex == 0 ? GameMode::SinglePlayer : GameMode::CoOp);

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
