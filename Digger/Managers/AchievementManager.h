#ifndef ACHIEVEMENTMANAGER_H
#define ACHIEVEMENTMANAGER_H

#include "Observer.h"

// Disable specific compiler warnings related to legacy Steam SDK macros so the project builds cleanly
#if USE_STEAMWORKS
#pragma warning (push)
#pragma warning (disable:4996)
#include <steam_api.h>
#pragma warning (pop)
#endif

// DESIGN PATTERN - Observer Pattern
// Using the Observer pattern prevents tight coupling. The AchievementManager listens passively to  event broadcasts (like "ScoreChanged"). This means the core gameplay loop doesn't have to  #include Steam APIs, which would completely break Emscripten web builds!

namespace dae
{
    class AchievementManager : public Observer
    {
    public:
        AchievementManager();

        void OnNotify(EventId eventId, int value) override;
        void ResetAchievements();

    private:
        // Conditional compilation macros (#if) ensure that Steam-specific logic is entirely stripped  from the executable when compiling for the web browser.
#if USE_STEAMWORKS
        STEAM_CALLBACK(AchievementManager, OnUserStatsReceived, UserStatsReceived_t, m_CallbackUserStatsReceived);
        bool m_bInitialized = false;
#endif

        bool m_WinnerUnlocked = false;
        void UnlockAchievement(const char* id);
    };
}

#endif // ACHIEVEMENTMANAGER_H