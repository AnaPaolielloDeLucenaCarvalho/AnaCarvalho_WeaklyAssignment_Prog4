#ifndef UIOBSERVERS_H
#define UIOBSERVERS_H

#include "Observer.h"
#include "Component.h"
#include "Subject.h"
#include "GameObject.h"
#include "Texture2D.h"

#include <string>
#include <vector>
#include <memory>
#include <SDL3/SDL.h>

namespace dae
{
    // LivesDisplayComponent — text-based lives counter
    class LivesDisplayComponent : public Component, public Observer
    {
    public:
        LivesDisplayComponent(GameObject* pOwner, const std::string& prefix, int startLives);

        void Update(float deltaTime) override;
        void OnNotify(EventId eventId, int value) override;

    private:
        std::string m_Prefix;
        int m_lives;
    };

    // LivesSpriteDisplayComponent — sprite-based lives display
    class LivesSpriteDisplayComponent : public Component, public Observer
    {
    public:
        LivesSpriteDisplayComponent(GameObject* pOwner, const std::string& texturePath, int startLives, float spacing = 35.f);

        void Update(float deltaTime) override;
        void Render() const override;
        void OnNotify(EventId eventId, int value) override;

    private:
        std::shared_ptr<Texture2D> m_texture;
        int m_lives;
        float m_spacing;
    };

    // ScoreDisplayComponent — text-based score display
    class ScoreDisplayComponent : public Component, public Observer
    {
    public:
        ScoreDisplayComponent(GameObject* pOwner, const std::string& prefix);

        void AddObserver(Observer* obs);
        void Update(float deltaTime) override;
        void OnNotify(EventId eventId, int value) override;

    private:
        std::string m_Prefix;
        int m_score;
        Subject m_Subject;
    };

    // SpriteScoreDisplayComponent — digit-sprite score display
    class SpriteScoreDisplayComponent : public Component, public Observer
    {
    public:
        SpriteScoreDisplayComponent(GameObject* pOwner, const std::string& digitPrefix = "VNUM", const std::string& extension   = ".png", float spacing = 24.f);

        void AddObserver(Observer* obs);
        void Update(float deltaTime) override;
        void Render() const override;
        void OnNotify(EventId eventId, int value) override;

    private:
        std::vector<std::shared_ptr<Texture2D>> m_digitTextures;
        int m_score;
        float m_spacing;
        Subject m_Subject;
    };

    // UIPanelComponent — filled-rectangle UI panel
    class UIPanelComponent : public Component
    {
    public:
        UIPanelComponent(GameObject* pOwner, float width, float height, SDL_Color color = { 0, 0, 0, 255 });

        void Update(float deltaTime) override;
        void Render() const override;

    private:
        float m_width;
        float m_height;
        SDL_Color m_color;
    };
}

#endif // UIOBSERVERS_H