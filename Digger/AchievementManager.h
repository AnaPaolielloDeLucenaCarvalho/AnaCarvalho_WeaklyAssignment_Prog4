#ifndef ACHIEVEMENTMANAGER_H
#define ACHIEVEMENTMANAGER_H

#include "Observer.h"

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
        AchievementManager();

        void OnNotify(EventId eventId, int value) override;
        void ResetAchievements();

    private:
        #if USE_STEAMWORKS
            STEAM_CALLBACK(AchievementManager, OnUserStatsReceived, UserStatsReceived_t, m_CallbackUserStatsReceived);
            bool m_bInitialized = false;
        #endif

        bool m_WinnerUnlocked = false;

        void UnlockAchievement(const char* id);
    };
}

#endif // ACHIEVEMENTMANAGER_H