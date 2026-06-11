#ifndef HIGH_SCORE_MANAGER_H
#define HIGH_SCORE_MANAGER_H

#include <string>
#include <vector>

namespace dae
{
    //  high-score entry (name + points).
    struct ScoreEntry
    {
		std::string initials; // always exactly 3 char - "ANA", "BOB", "AAA"
        int score;
    };

    class HighScoreManager final
    {
    public:
        HighScoreManager();
        ~HighScoreManager();

        HighScoreManager(const HighScoreManager&) = delete;
        HighScoreManager& operator=(const HighScoreManager&) = delete;
        HighScoreManager(HighScoreManager&&) = delete;
        HighScoreManager& operator=(HighScoreManager&&) = delete;

        bool HasSessionName() const;

        void SetSessionName(const std::string& initials);

        const std::string& GetSessionName() const;

        std::vector<ScoreEntry> GetTopScores(int count) const;

        void UpdateCurrentScore(int score);

        void SaveScore(int score);

    private:
        static std::string NormaliseInitials(const std::string& raw);

        std::string m_sessionName; // 3-char; empty = not set yet
        bool m_nameIsSet; // avoids empty-string ambiguity
        int m_currentScore{0};
        bool m_hasSaved{false};
    };

}

#endif // HIGH_SCORE_MANAGER_H
