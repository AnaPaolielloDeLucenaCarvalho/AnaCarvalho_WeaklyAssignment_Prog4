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
        // Game Over - wipe the active session name so the next game prompts for initials again
        if (m_pHighScoreManager)
        {
            m_pHighScoreManager->ClearSessionName();
        }

        // Force a hard reset (restoring lives and state) before swapping scenes
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
        // Debounce Logic - using SDL_GetTicks instead of deltaTime, because Commands only execute on the exact frame a key is pressed,  deltaTime would force the user to manually mash the button to tick the timer down.
        uint64_t currentTime = SDL_GetTicks();
        if (currentTime - m_LastPressTime < 300) return;
        m_LastPressTime = currentTime;

        auto& sm = SceneManager::GetInstance();
        Scene* currentScene = sm.GetActiveScene();

        // Toggle instructions scene
        if (currentScene == m_pInstructionsScene)
        {
            // Close instructions, go back to saved scene, unpause audio
            if (m_pPreviousScene) sm.SetActiveScene(m_pPreviousScene);
            ServiceLocator::GetSoundSystem().ResumeMusic();
        }
        else
        {
            // Open instructions, save the current scene, pause audio
            // Saving the previous scene allows us to pause the game safely without losing state
            m_pPreviousScene = currentScene;
            sm.SetActiveScene(m_pInstructionsScene);
            ServiceLocator::GetSoundSystem().PauseMusic();
        }
    }

    void MuteCommand::Execute(float /*deltaTime*/)
    {
        // Absolute cooldown to prevent the mute toggle from flickering on and off
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
        // 300ms absolute cooldown to prevent rapid-fire level skipping
        uint64_t currentTime = SDL_GetTicks();
        if (currentTime - m_LastPressTime < 300) return;
        m_LastPressTime = currentTime;

        // DESIGN PATTERN - Observer Pattern
        // Instead of hard-linking the level skip to the LevelManager, we notify the Player's Subject. Observer pattern ensures the command shouts "LoadNextLevel", and the listening manager handles it without tight coupling
        if (m_pDigger)
        {
            m_pDigger->GetSubject().Notify(make_sdbm_hash("LoadNextLevel"), 0);
        }
    }
}
