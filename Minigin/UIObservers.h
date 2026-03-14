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

        LivesDisplayComponent(GameObject* pOwner, int startLives) : Component(pOwner), m_lives(startLives) {}

        void Update(float /*deltaTime*/) override {}


        void OnNotify(Event event, int value) override
        {
            if (event == Event::PlayerDied)
            {
                if (auto text = GetOwner()->GetComponent<TextComponent>())
                {
                    text->SetText("Lives: " + std::to_string(value));
                }
            }
        }
    private:
        int m_lives;
    };

    class ScoreDisplayComponent : public Component, public Observer
    {
    public:
        ScoreDisplayComponent(GameObject* pOwner) : Component(pOwner), m_score(0) {}

        void Update(float /*deltaTime*/) override {}

        void OnNotify(Event event, int value) override
        {
            if (event == Event::DiamondPickedUp)
            {
                if (auto text = GetOwner()->GetComponent<TextComponent>())
                {
                    text->SetText("Score: " + std::to_string(value));
                }
            }
        }
	private:
		int m_score;
    };
}
#endif