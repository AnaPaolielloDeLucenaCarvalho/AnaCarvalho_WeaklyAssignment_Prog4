#ifndef ACHIEVEMENTMANAGER_H
#define ACHIEVEMENTMANAGER_H

#include "Observer.h"
#include <iostream>

#if USE_STEAMWORKS
#pragma warning (push)
#pragma warning (disable:4996)
#include <steam_api.h>
#pragma warning (pop)
#endif

namespace dae
{
    class AchievementManager : public Observer
    {
    public:
        AchievementManager()
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

        void OnNotify(EventId eventId, int value) override
        {
            if (eventId == make_sdbm_hash("ScoreChanged"))
            {
                // feedback - avoid spamming
                if (value >= 500 && !m_WinnerUnlocked)
                {
                    bool achieved = false;
                    if (SteamUserStats() && SteamUserStats()->GetAchievement("ACH_WIN_ONE_GAME", &achieved))
                    {
                        if (!achieved)
                        {
                            UnlockAchievement("ACH_WIN_ONE_GAME");
                        }
                        m_WinnerUnlocked = true; // Lock it locally so we don't check again
                    }
                }
            }
        }

        void ResetAchievements()
        {
            #if USE_STEAMWORKS
                if (SteamUserStats())
                {
                    SteamUserStats()->ClearAchievement("ACH_WIN_ONE_GAME");
                    SteamUserStats()->StoreStats();
                    m_WinnerUnlocked = false;
                    std::cout << "Steam Achievements Reset!" << std::endl;
                }
            #endif
        }

    private:
        #if USE_STEAMWORKS
            STEAM_CALLBACK(AchievementManager, OnUserStatsReceived, UserStatsReceived_t, m_CallbackUserStatsReceived);
            bool m_bInitialized = false;
        #endif

        bool m_WinnerUnlocked = false;

        void UnlockAchievement(const char* id)
        {
            #if USE_STEAMWORKS
                if (m_bInitialized && SteamUserStats())
                {
                    SteamUserStats()->SetAchievement(id);
                    SteamUserStats()->StoreStats();
                }
            #else
                (void)id;
            #endif
        }
    };

    #if USE_STEAMWORKS
        inline void AchievementManager::OnUserStatsReceived(UserStatsReceived_t* pCallback)
        {
            if (pCallback->m_eResult == k_EResultOK)
            {
                m_bInitialized = true;
                bool achieved = false;
                if (SteamUserStats()->GetAchievement("ACH_WIN_ONE_GAME", &achieved))
                {
                    m_WinnerUnlocked = achieved;
                }
            }
        }
    #endif
}
#endif