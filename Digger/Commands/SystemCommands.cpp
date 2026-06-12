#include "SystemCommands.h"
#include "SceneManager.h"
#include "LevelManager.h"
#include "ServiceLocator.h"
#include <SDL3/SDL.h>
#include "HighScoreManager.h"
#include "DiggerComponent.h"
#include "Observer.h"

namespace dae
{
    ReturnToMenuCommand::ReturnToMenuCommand(Scene* menuScene, HighScoreManager* highScoreManager)
        : m_pMenuScene(menuScene)
        , m_pHighScoreManager(highScoreManager)
    {
    }

    void ReturnToMenuCommand::Execute(float /*deltaTime*/)
    {
        if (m_pHighScoreManager)
        {
            m_pHighScoreManager->ClearSessionName();
        }
        LevelManager::GetInstance().SetNeedsGameReset(true);
        SceneManager::GetInstance().SetActiveScene(m_pMenuScene);
        ServiceLocator::GetSoundSystem().ResumeMusic();
    }

    ToggleInstructionsCommand::ToggleInstructionsCommand(Scene* instructionsScene)
        : m_pInstructionsScene(instructionsScene)
    {
    }

    void ToggleInstructionsCommand::Execute(float /*deltaTime*/)
    {
        uint64_t currentTime = SDL_GetTicks();
        if (currentTime - m_LastPressTime < 300) return;
        m_LastPressTime = currentTime;

        auto& sm = SceneManager::GetInstance();
        Scene* currentScene = sm.GetActiveScene();

        if (currentScene == m_pInstructionsScene)
        {
            // Close instructions, go back to saved scene, unpause audio
            if (m_pPreviousScene) sm.SetActiveScene(m_pPreviousScene);
            ServiceLocator::GetSoundSystem().ResumeMusic();
        }
        else
        {
            // Open instructions, save the current scene, pause audio
            m_pPreviousScene = currentScene;
            sm.SetActiveScene(m_pInstructionsScene);
            ServiceLocator::GetSoundSystem().PauseMusic();
        }
    }
    void MuteCommand::Execute(float /*deltaTime*/)
    {
        const uint64_t currentTime = SDL_GetTicks();
        if (currentTime - m_LastToggleTime > 300)
        {
            ServiceLocator::GetSoundSystem().ToggleMute();
            m_LastToggleTime = currentTime;
        }
    }

    SkipLevelCommand::SkipLevelCommand(DiggerComponent* pDigger)
        : m_pDigger(pDigger)
    {
    }

    void SkipLevelCommand::Execute(float /*deltaTime*/)
    {
        uint64_t currentTime = SDL_GetTicks();
        if (currentTime - m_LastPressTime < 300) return;
        m_LastPressTime = currentTime;

        if (m_pDigger)
        {
            m_pDigger->GetSubject().Notify(make_sdbm_hash("LoadNextLevel"), 0);
        }
    }
}
