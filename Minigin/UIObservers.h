#ifndef UIOBSERVERS_H
#define UIOBSERVERS_H

#include "Observer.h"
#include "Component.h"
#include "TextComponent.h"
#include "GameObject.h"
#include <string>

namespace dae
{
    class LivesDisplayComponent : public Component, public Observer
    {
    public:

		// feedback - add prefix
        LivesDisplayComponent(GameObject* pOwner, const std::string& prefix, int startLives) 
            : Component(pOwner)
            , m_Prefix(prefix)
            , m_lives(startLives) 
        {
        }

        void Update(float /*deltaTime*/) override {}

        void OnNotify(EventId eventId, int value) override
        {
            if (eventId == make_sdbm_hash("PlayerDied"))
            {
                m_lives = value;

                if (auto text = GetOwner()->GetComponent<TextComponent>())
                {
                    text->SetText(m_Prefix + "Lives: " + std::to_string(m_lives));
                }
            }
        }

    private:
        std::string m_Prefix;
        int m_lives;
    };

    class ScoreDisplayComponent : public Component, public Observer
    {
    public:
        ScoreDisplayComponent(GameObject* pOwner, const std::string& prefix)
            : Component(pOwner), m_Prefix(prefix), m_score(0) {
        }

        void AddObserver(Observer* obs) { m_Subject.AddObserver(obs); }

        void Update(float /*deltaTime*/) override {}

        void OnNotify(EventId eventId, int value) override
        {
            if (eventId == make_sdbm_hash("DiamondPickedUp"))
            {
                m_score += value;

                if (auto text = GetOwner()->GetComponent<TextComponent>())
                {
                    text->SetText(m_Prefix + "Score: " + std::to_string(m_score));
                }
                m_Subject.Notify(make_sdbm_hash("ScoreChanged"), m_score);
            }
        }

    private:
        std::string m_Prefix;
        int m_score;
        Subject m_Subject;
    };
}
#endif