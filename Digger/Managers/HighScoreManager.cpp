#include "HighScoreManager.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <SDL3/SDL_messagebox.h>

namespace dae
{

    HighScoreManager::HighScoreManager()
        : m_sessionName{ "AAA" }
        , m_nameIsSet{ false }
    {
    }

    HighScoreManager::~HighScoreManager()
    {
        // Failsafe - Ensures that if the window is forcefully closed before the Game Over screen is reached, the player's active session score is still safely dumped to the text file.
        if (m_nameIsSet && !m_hasSaved)
        {
            SaveScore(m_currentScore);
        }
    }

    bool HighScoreManager::HasSessionName() const
    {
        return m_nameIsSet;
    }

    void HighScoreManager::SetSessionName(const std::string& initials)
    {
        m_sessionName = NormaliseInitials(initials);
        m_nameIsSet = true;
    }

    void HighScoreManager::ClearSessionName()
    {
        m_nameIsSet = false;
        m_sessionName = "AAA";
        m_hasSaved = false; // Extremely important so the next score saves!
    }

    const std::string& HighScoreManager::GetSessionName() const
    {
        return m_sessionName;
    }

    std::vector<ScoreEntry> HighScoreManager::GetTopScores(int count) const
    {
        namespace fs = std::filesystem;

        // Conditional logic guarantees that the file path resolves correctly whether  running natively on Windows/Mac or packaged in a Web Browser via Emscripten.
#ifdef __EMSCRIPTEN__
        const fs::path filePath = "highscores.txt";
#else
        fs::path dataDir = "./Data";
        if (!fs::exists(dataDir))
        {
            dataDir = "../Data";
        }
        const fs::path filePath = dataDir / "highscores.txt";
#endif

        std::vector<ScoreEntry> entries;
        std::ifstream ifs(filePath);
        if (ifs.is_open())
        {
            // Parse the text file line by line, mapping strings to integers into the struct
            std::string line;
            while (std::getline(ifs, line))
            {
                if (line.empty()) continue;
                std::istringstream iss(line);
                std::string initials;
                int score;
                if (iss >> initials >> score)
                {
                    entries.push_back({ initials, score });
                }
            }
        }

        // Sort descending using a lambda function to order the struct by integer value
        std::sort(entries.begin(), entries.end(), [](const ScoreEntry& a, const ScoreEntry& b) {
            return a.score > b.score;
            });

        // Truncate the list to the requested display amount (e.g., Top 10)
        if (entries.size() > static_cast<size_t>(count))
        {
            entries.resize(count);
        }

        // Fill empty slots with dummy variables so the UI never attempts to render a null position
        char dummyChar = 'A';
        while (entries.size() < static_cast<size_t>(count))
        {
            std::string dummyName(3, dummyChar);
            entries.push_back({ dummyName, 0 });
            if (dummyChar < 'Z') dummyChar++;
        }

        return entries;
    }

    void HighScoreManager::UpdateCurrentScore(int score)
    {
        m_currentScore = score;
    }

    void HighScoreManager::SaveScore(int score)
    {
        namespace fs = std::filesystem;

#ifdef __EMSCRIPTEN__
        const fs::path filePath = "highscores.txt";
#else
        fs::path dataDir = "./Data";
        if (!fs::exists(dataDir))
        {
            dataDir = "../Data";
        }

        if (!fs::exists(dataDir))
        {
            fs::create_directories(dataDir);
        }

        const fs::path filePath = dataDir / "highscores.txt";
#endif

        // Open the file string in append mode (ios::app) so we don't accidentally overwrite historical scores
        std::ofstream ofs(filePath, std::ios::app);
        if (ofs.is_open())
        {
            ofs << m_sessionName << " " << score << "\n";
            m_hasSaved = true;

            //std::string successMsg = "Score saved securely to:\n" + fs::absolute(filePath).string();
            //SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Save Success", successMsg.c_str(), nullptr);
        }
        else
        {
            //std::string errorMsg = "Failed to open file stream at:\n" + fs::absolute(filePath).string();
            //SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Save Error", errorMsg.c_str(), nullptr);
        }
    }

    std::string HighScoreManager::NormaliseInitials(const std::string& raw)
    {
        std::string result = raw;

        // Force all lowercase letters to uppercase to standardize visual rendering
        std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });

        // Pad the string with 'A's to ensure it meets the rigid 3-character format
        while (result.size() < 3) result += 'A';

        result.resize(3);

        return result;
    }

}