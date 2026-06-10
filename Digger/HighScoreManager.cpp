#include "HighScoreManager.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <filesystem>
#include <SDL3/SDL_messagebox.h>

namespace dae
{

    HighScoreManager::HighScoreManager()
        : m_sessionName{ "AAA" }
        , m_nameIsSet  { false  }
    {
    }

    HighScoreManager::~HighScoreManager()
    {
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
        m_nameIsSet   = true;
    }

    const std::string& HighScoreManager::GetSessionName() const
    {
        return m_sessionName;
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

        std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });

        while (result.size() < 3) result += 'A';

        result.resize(3);

        return result;
    }

}
