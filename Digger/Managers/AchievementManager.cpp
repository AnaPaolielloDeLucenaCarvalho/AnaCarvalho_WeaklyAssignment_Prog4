#include "AchievementManager.h"

#include <iostream>

namespace dae
{
    AchievementManager::AchievementManager()
#if USE_STEAMWORKS
        : m_callbackUserStatsReceived(this, &AchievementManager::OnUserStatsReceived)
#endif
    {
#if USE_STEAMWORKS
        // Validate that the Steam client is actively running before requesting user statistics
        if (SteamUserStats() && SteamUser())
        {
            SteamUserStats()->RequestUserStats(SteamUser()->GetSteamID());
        }
#endif
    }

    void AchievementManager::OnNotify(EventId eventId, int value)
    {
        if (eventId == make_sdbm_hash("ScoreChanged"))
        {
            // Gate condition: Once the player exceeds 500 points, unlock the achievement if not already awarded
            if (value >= 500 && !m_winnerUnlocked)
            {
#if USE_STEAMWORKS
                bool achieved = false;
                if (SteamUserStats() &&
                    SteamUserStats()->GetAchievement("ACH_WIN_ONE_GAME", &achieved))
                {
                    if (!achieved)
                    {
                        UnlockAchievement("ACH_WIN_ONE_GAME");
                    }
                    m_winnerUnlocked = true;
                }
#else
                // Safe fallback logic for Web Builds so the console proves the event fired correctly
                m_winnerUnlocked = true;
                std::cout << "Local Achievement Unlocked! (Steam disabled)\n";
#endif
            }
        }
    }

    void AchievementManager::ResetAchievements()
    {
#if USE_STEAMWORKS
        if (SteamUserStats())
        {
            // Clear the achievement locally and push the updated state securely to the Steam backend
            SteamUserStats()->ClearAchievement("ACH_WIN_ONE_GAME");
            SteamUserStats()->StoreStats();
            m_winnerUnlocked = false;
            std::cout << "Steam Achievements Reset!\n";
        }
#endif
    }

    void AchievementManager::UnlockAchievement(const char* id)
    {
#if USE_STEAMWORKS
        if (m_bInitialized && SteamUserStats())
        {
            // Register the achievement locally and push the updated state securely to the Steam backend
            SteamUserStats()->SetAchievement(id);
            SteamUserStats()->StoreStats();
        }
#else
        (void)id; // suppress unused-parameter warning in non-Steam builds
#endif
    }

#if USE_STEAMWORKS
    // Callback function fired asynchronously when the Steam backend replies with the user's data
    void AchievementManager::OnUserStatsReceived(UserStatsReceived_t* pCallback)
    {
        if (pCallback->m_eResult == k_EResultOK)
        {
            m_bInitialized = true;
            bool achieved = false;
            // Pre-load the achievement state to ensure we don't spam the unlock API unnecessarily
            if (SteamUserStats()->GetAchievement("ACH_WIN_ONE_GAME", &achieved))
            {
                m_winnerUnlocked = achieved;
            }
        }
    }
#endif
}
