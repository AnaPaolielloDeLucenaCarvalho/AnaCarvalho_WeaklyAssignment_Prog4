#ifndef GAMEOVERMANAGER_H
#define GAMEOVERMANAGER_H

#include "Component.h"
#include <vector>
#include <string>

// DESIGN PATTERN: Component Pattern
// Rather than hardcoding the Game Over menu logic directly into the Scene, this Manager is attached to an empty GameObject. It dictates UI flow, handles user selection, and bridges communication between the text elements and the HighScoreManager.

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

        void Render() const override {}
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
        std::vector<TextComponent*> m_options;

        bool m_isSetup{ false };
        int m_selectedIndex{ 0 };
    };
}
#endif
