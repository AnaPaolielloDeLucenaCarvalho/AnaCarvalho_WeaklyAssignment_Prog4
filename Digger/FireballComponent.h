#ifndef FIREBALLCOMPONENT_H
#define FIREBALLCOMPONENT_H

#include "Component.h"
#include "GameObject.h"
#include "RenderComponent.h"
#include "LevelManager.h"
#include <glm/glm.hpp>
#include <string>

namespace dae
{
    class FireballComponent : public Component
    {
    public:
        FireballComponent(GameObject* owner, const glm::vec2& dir)
            : Component(owner), m_Direction(dir)
        {
        }

        void Update(float deltaTime) override
        {
			// explosion logic (if exploding, ignore movement and animate explosion instead)
            if (m_IsExploding)
            {
                m_ExplodeTimer += deltaTime;
                if (m_ExplodeTimer > 0.1f) // 0.1s per explosion
                {
                    m_ExplodeTimer -= 0.1f;
                    m_ExplodeFrame++;

                    if (m_ExplodeFrame > 3)
                    {
						GetOwner()->MarkForDestroy(); // destroy
                    }
                    else if (auto render = GetOwner()->GetComponent<RenderComponent>())
                    {
                        render->SetTexture("PNG/Other/VEXP" + std::to_string(m_ExplodeFrame) + ".png");
                    }
                }
                return; // freeze in place
            }

			// shooting logic
            float speed = 300.0f * deltaTime;
            auto pos = GetOwner()->GetTransform().GetPosition();
            float newX = pos.x + (m_Direction.x * speed);
            float newY = pos.y + (m_Direction.y * speed);

            GetOwner()->SetLocalPosition(newX, newY);

			// fireball animation
            m_AnimTimer += deltaTime;
            if (m_AnimTimer > 0.1f)
            {
                m_AnimTimer -= 0.1f;
                m_Frame++;
                if (m_Frame > 3) m_Frame = 1;

                if (auto render = GetOwner()->GetComponent<RenderComponent>())
                {
                    render->SetTexture("PNG/Other/VFIRE" + std::to_string(m_Frame) + ".png");
                }
            }

			// TODO - kill enemies in path here

			// destroy fireball if hits dirt or out of bounds
            if (LevelManager::GetInstance().IsDirtAt(newX, newY) ||
                newX < 0 || newX > 1040 || newY < 0 || newY > 612)
            {
                m_IsExploding = true;
                if (auto render = GetOwner()->GetComponent<RenderComponent>())
                {
					render->SetTexture("PNG/Other/VEXP1.png"); // explosion for collision
                }
            }
        }

    private:
        glm::vec2 m_Direction;
        float m_AnimTimer{ 0.0f };
        int m_Frame{ 1 };

		// explosion state
        bool m_IsExploding{ false };
        float m_ExplodeTimer{ 0.0f };
        int m_ExplodeFrame{ 1 };
    };
}
#endif