#include "AchievementManager.h"

#include <iostream>

namespace dae
{
    AchievementManager::AchievementManager()
#if USE_STEAMWORKS
        : m_CallbackUserStatsReceived(this, &AchievementManager::OnUserStatsReceived)
#endif
    {
#if USE_STEAMWORKS
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
            if (value >= 500 && !m_WinnerUnlocked)
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
                    m_WinnerUnlocked = true;
                }
#else
                m_WinnerUnlocked = true;
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
            SteamUserStats()->ClearAchievement("ACH_WIN_ONE_GAME");
            SteamUserStats()->StoreStats();
            m_WinnerUnlocked = false;
            std::cout << "Steam Achievements Reset!\n";
        }
#endif
    }

    void AchievementManager::UnlockAchievement(const char* id)
    {
#if USE_STEAMWORKS
        if (m_bInitialized && SteamUserStats())
        {
            SteamUserStats()->SetAchievement(id);
            SteamUserStats()->StoreStats();
        }
#else
        (void)id; // suppress unused-parameter warning in non-Steam builds
#endif
    }

#if USE_STEAMWORKS
    void AchievementManager::OnUserStatsReceived(UserStatsReceived_t* pCallback)
    {
        if (pCallback->m_eResult == k_EResultOK)
        {
            m_bInitialized = true;
            bool achieved  = false;
            if (SteamUserStats()->GetAchievement("ACH_WIN_ONE_GAME", &achieved))
            {
                m_WinnerUnlocked = achieved;
            }
        }
    }
#endif
}
