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
            : m_CallbackUserStatsReceived(this, &AchievementManager::OnUserStatsReceived)
        {
            #if USE_STEAMWORKS
                if (SteamUserStats() && SteamUser())
                {
                    SteamUserStats()->RequestUserStats(SteamUser()->GetSteamID());
                    std::cout << "AchievementManager: Steam Handshake Started." << std::endl;
                }
                else
                {
                    std::cout << "AchievementManager ERROR: Steam interfaces not available yet!" << std::endl;
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
                    bool success = SteamUserStats()->StoreStats();

                    if (success)
                    {
                        std::cout << "Steam: SetAchievement + StoreStats successful for: " << id << std::endl;
                    }
                }
                else if (!m_bInitialized)
                {
                    std::cout << "Steam: Cannot unlock yet, waiting for m_bInitialized..." << std::endl;
                }
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
                std::cout << "Steam Stats synced successfully! Achievement system initialized." << std::endl;
                m_bInitialized = true;

                bool achieved = false;
                if (SteamUserStats()->GetAchievement("ACH_WIN_ONE_GAME", &achieved))
                {
                    m_WinnerUnlocked = achieved;
                    if (achieved) std::cout << "Status: ACH_WIN_ONE_GAME is already UNLOCKED." << std::endl;
                }
            }
            else
            {
                std::cout << "Steam Stats sync failed with error: " << pCallback->m_eResult << std::endl;
            }
        }
    #endif
}
#endif