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

        void OnNotify(Event event, int value) override
        {
            if (event == Event::DiamondPickedUp)
            {
                if (value >= 400 && !m_WinnerUnlocked)
                {
                    UnlockAchievement("ACH_WIN_ONE_GAME");
                    m_WinnerUnlocked = true;
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
        bool m_bInitialized = false;
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

        #if USE_STEAMWORKS
            STEAM_CALLBACK(AchievementManager, OnUserStatsReceived, UserStatsReceived_t, m_CallbackUserStatsReceived);
        #endif
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