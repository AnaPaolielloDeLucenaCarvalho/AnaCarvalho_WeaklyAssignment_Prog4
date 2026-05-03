#include "GoldBagComponent.h"
#include "GoldBagState.h"
#include "LevelManager.h"
#include "DiggerComponent.h"
#include "DiggerState.h"
#include <cmath>

namespace dae
{
    GoldBagComponent::GoldBagComponent(dae::GameObject* owner) : dae::Component(owner)
    {
        m_pCurrentState = new GoldBagIdleState();
        m_pCurrentState->OnEnter(this);
    }

    GoldBagComponent::~GoldBagComponent()
    {
        if (m_pCurrentState)
        {
            m_pCurrentState->OnExit(this);
            delete m_pCurrentState;
        }
    }

    void GoldBagComponent::Update(float deltaTime)
    {
        if (m_pCurrentState)
        {
            GoldBagState* newState = m_pCurrentState->Update(this, deltaTime);
            if (newState != nullptr)
            {
                ChangeState(newState);
            }
        }
    }

    void GoldBagComponent::ChangeState(GoldBagState* newState)
    {
        if (m_pCurrentState)
        {
            m_pCurrentState->OnExit(this);
            delete m_pCurrentState;
        }

        m_pCurrentState = newState;

        if (m_pCurrentState)
        {
            m_pCurrentState->OnEnter(this);
        }
    }

    bool GoldBagComponent::IsDirtDirectlyUnderneath() const
    {
        auto myPos = GetOwner()->GetTransform().GetPosition();

        return LevelManager::GetInstance().IsDirtAt(myPos.x, myPos.y + 40.0f);
    }

    bool GoldBagComponent::HitBottom() const
    {
        auto myPos = GetOwner()->GetTransform().GetPosition();
        return (myPos.y >= 540.f);
    }

    void GoldBagComponent::SmashEntitiesBelow()
    {
        auto myPos = GetOwner()->GetTransform().GetPosition();

        auto checkAndSmash = [&](GameObject* player) {
            if (player && !player->IsMarkedForDestroy())
            {
                auto pPos = player->GetTransform().GetPosition();

                if (std::abs(pPos.x - myPos.x) < 20.f && (pPos.y - myPos.y) > 0 && (pPos.y - myPos.y) < 30.f)
                {
                    if (auto diggerComp = player->GetComponent<DiggerComponent>())
                    {
                        if (!diggerComp->IsDead() && diggerComp->GetLives() > 0)
                        {
                            diggerComp->ChangeState(new DiggerDeadState());
                        }
                    }
                }
            }
            };

        checkAndSmash(m_pPlayer1);
        checkAndSmash(m_pPlayer2);
    }
}