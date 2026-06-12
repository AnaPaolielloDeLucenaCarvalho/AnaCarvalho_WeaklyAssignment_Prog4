#include "CherryComponent.h"
#include "DiggerComponent.h"
#include "ServiceLocator.h"
#include "AudioDefinitions.h"

#include <glm/glm.hpp>
#include <glm/geometric.hpp>

namespace dae
{
    CherryComponent::CherryComponent(GameObject* owner, DiggerComponent* p1, DiggerComponent* p2, float lifetime)
        : Component(owner)
        , m_p1(p1)
        , m_p2(p2)
        , m_Lifetime(lifetime)
    {
    }

    bool CherryComponent::CheckPickup(DiggerComponent* pDigger, float myX, float myY) const
    {
        // Safe pointer check and early-out if the player is already dead or already has the bonus active
        if (!pDigger || pDigger->IsDead() || pDigger->IsInBonusMode()) return false;

        // Simple radius-based collision detection using Pythagorean distance
        const auto pos = pDigger->GetOwner()->GetTransform().GetPosition();
        if (glm::distance(glm::vec2(myX, myY), glm::vec2(pos.x, pos.y)) < 30.f)
        {
            pDigger->ActivateBonusMode();
            ServiceLocator::GetSoundSystem().Play(AudioDefinitions::BONUS_PICKUP, 1.0f);
            return true;
        }
        return false;
    }

    void CherryComponent::Update(float deltaTime)
    {
        // Tick down the cherry's lifespan so it despawns if the player ignores it
        m_Lifetime -= deltaTime;
        if (m_Lifetime <= 0.0f)
        {
            GetOwner()->MarkForDestroy();
            return;
        }

        const auto myPos = GetOwner()->GetTransform().GetPosition();
        const float myX = myPos.x;
        const float myY = myPos.y;

        // Check both players — first player to reach it activates bonus mode and consumes the cherry
        if (CheckPickup(m_p1, myX, myY) || CheckPickup(m_p2, myX, myY))
        {
            GetOwner()->MarkForDestroy();
        }
    }
}
