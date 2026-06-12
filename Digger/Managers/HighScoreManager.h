#ifndef HIGH_SCORE_MANAGER_H
#define HIGH_SCORE_MANAGER_H

#include <string>
#include <vector>

// DESIGN PATTERN: Architectural Separation
// The HighScoreManager is completely isolated from GameObjects. It is a pure data manager  responsible for disk I/O (saving to text files) and sorting arrays. It is passed via  dependency injection rather than acting as a rigid global Singleton.

namespace dae
{
    // High-score entry structure to rigidly lock a 3-character name string to an integer point value.
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
        void ClearSessionName();

        const std::string& GetSessionName() const;

        std::vector<ScoreEntry> GetTopScores(int count) const;

        void UpdateCurrentScore(int score);
        int GetCurrentScore() const { return m_currentScore; }
        void SetCurrentScore(int score) { m_currentScore = score; }

        void SaveScore(int score);

    private:
        static std::string NormaliseInitials(const std::string& raw);

        std::string m_sessionName; // 3-char; empty = not set yet
        bool m_nameIsSet; // avoids empty-string ambiguity
        int m_currentScore{ 0 };
        bool m_hasSaved{ false };
    };

}

#endif // HIGH_SCORE_MANAGER_H
