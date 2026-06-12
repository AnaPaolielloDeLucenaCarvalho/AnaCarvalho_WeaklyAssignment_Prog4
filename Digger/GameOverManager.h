#ifndef GAMEOVERMANAGER_H
#define GAMEOVERMANAGER_H

#include "Component.h"
#include <vector>
#include <string>

namespace dae
{
    class Scene;
    class TextComponent;
    class HighScoreManager;

    class GameOverManager final : public Component
    {
    public:
        GameOverManager(GameObject* pOwner, Scene* menuScene, Scene* gameScene, HighScoreManager* highScoreMgr, TextComponent* titleText, TextComponent* scoreText, const std::vector<TextComponent*>& options);
        ~GameOverManager() override = default;

        GameOverManager(const GameOverManager& other) = delete;
        GameOverManager(GameOverManager&& other) = delete;
        GameOverManager& operator=(const GameOverManager& other) = delete;
        GameOverManager& operator=(GameOverManager&& other) = delete;

        void Update(float deltaTime) override;

        void NavigateUp();
        void NavigateDown();
        void Select();

    private:
        Scene* m_pMenuScene;
        Scene* m_pGameScene;
        HighScoreManager* m_pHighScoreMgr;
        
        TextComponent* m_pTitleText;
        TextComponent* m_pScoreText;
        std::vector<TextComponent*> m_Options;

        bool m_IsSetup{false};
        int m_SelectedIndex{0};
    };
}
#endif
